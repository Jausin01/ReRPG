#include "AI/EnemyAIController.h"

#include "AI/EnemyInfoComponent.h"
#include "BrainComponent.h"
#include "Combat/CombatTargeting.h"
#include "Combat/HealthComponent.h"
#include "Combat/MeleeAttackComponent.h"
#include "Combat/StunComponent.h"
#include "Combat/TeamComponent.h"
#include "Combat/ThreatComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Navigation/PathFollowingComponent.h"
#include "TimerManager.h"

void AEnemyAIController::BeginPlay()
{
	// Find the first target and set the flags before the StateTree component starts (components begin inside Super).
	UpdatePerception();

	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(PerceptionTimer, this, &AEnemyAIController::UpdatePerception, PerceptionInterval, true);
}

void AEnemyAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(PerceptionTimer);

	// Release the listener on the target's health.
	SetTarget(nullptr);

	Super::EndPlay(EndPlayReason);
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Cache the combat components of the controlled pawn once.
	if (InPawn != nullptr)
	{
		PawnMelee = InPawn->FindComponentByClass<UMeleeAttackComponent>();
		PawnStun = InPawn->FindComponentByClass<UStunComponent>();
		PawnHealth = InPawn->FindComponentByClass<UHealthComponent>();
		PawnInfo = InPawn->FindComponentByClass<UEnemyInfoComponent>();
		PawnThreat = InPawn->FindComponentByClass<UThreatComponent>();
	}

	if (UStunComponent* Stun = PawnStun.Get())
	{
		bPawnStunned = Stun->IsStunned();
		Stun->OnStunStarted.AddDynamic(this, &AEnemyAIController::HandleStunStarted);
		Stun->OnStunEnded.AddDynamic(this, &AEnemyAIController::HandleStunEnded);
	}

	if (UHealthComponent* Health = PawnHealth.Get())
	{
		Health->OnDamageTaken.AddDynamic(this, &AEnemyAIController::HandleDamageTaken);
		Health->OnDeath.AddDynamic(this, &AEnemyAIController::HandleDeath);
	}
}

void AEnemyAIController::OnUnPossess()
{
	if (UStunComponent* Stun = PawnStun.Get())
	{
		Stun->OnStunStarted.RemoveDynamic(this, &AEnemyAIController::HandleStunStarted);
		Stun->OnStunEnded.RemoveDynamic(this, &AEnemyAIController::HandleStunEnded);
	}

	if (UHealthComponent* Health = PawnHealth.Get())
	{
		Health->OnDamageTaken.RemoveDynamic(this, &AEnemyAIController::HandleDamageTaken);
		Health->OnDeath.RemoveDynamic(this, &AEnemyAIController::HandleDeath);
	}

	// Without a controller nobody can keep the combat state up to date, so close it.
	if (UEnemyInfoComponent* Info = PawnInfo.Get())
	{
		Info->SetInCombat(false);
	}

	PawnMelee = nullptr;
	PawnStun = nullptr;
	PawnHealth = nullptr;
	PawnInfo = nullptr;
	PawnThreat = nullptr;
	bPawnStunned = false;

	Super::OnUnPossess();
}

AActor* AEnemyAIController::ResolveTarget() const
{
	// A dead target counts as no target, so every query below sees "nothing to chase".
	return (bTargetDead || !IsValid(TargetActor)) ? nullptr : TargetActor.Get();
}

AActor* AEnemyAIController::FindNearestHostile(float Radius) const
{
	const APawn* ControlledPawn = GetPawn();
	if (ControlledPawn == nullptr)
	{
		return nullptr;
	}

	const FVector Location = ControlledPawn->GetActorLocation();

	TArray<TWeakObjectPtr<UHealthComponent>> Candidates;
	CombatTargeting::FindHostileTargets(ControlledPawn, Location, Radius, Candidates);

	// The nearest living hostile pawn wins.
	AActor* Nearest = nullptr;
	float NearestDistance = TNumericLimits<float>::Max();
	for (const TWeakObjectPtr<UHealthComponent>& Candidate : Candidates)
	{
		const UHealthComponent* Health = Candidate.Get();
		AActor* Actor = Health != nullptr ? Health->GetOwner() : nullptr;

		// A health component that has not begun play still holds 0 health: do not read it as dead.
		if (Actor == nullptr || (Health->HasBegunPlay() && Health->IsDead()))
		{
			continue;
		}

		const float Distance = FVector::Dist2D(Location, Actor->GetActorLocation());
		if (Distance < NearestDistance)
		{
			NearestDistance = Distance;
			Nearest = Actor;
		}
	}

	return Nearest;
}

AActor* AEnemyAIController::FindNewTarget()
{
	const APawn* ControlledPawn = GetPawn();
	if (ControlledPawn == nullptr)
	{
		return nullptr;
	}

	// Someone who already hurt this enemy comes first.
	if (UThreatComponent* Threat = PawnThreat.Get())
	{
		if (AActor* Threatening = Threat->PickTarget(ControlledPawn->GetActorLocation(), GetGiveUpRange(), nullptr))
		{
			return Threatening;
		}
	}

	// Otherwise the nearest living hostile pawn wins.
	return FindNearestHostile(AggroRange);
}

void AEnemyAIController::SetTarget(AActor* NewTarget)
{
	// Moving to another living target (not just releasing the old one) may need a new move request.
	const bool bSwitchedToOtherTarget = NewTarget != nullptr && NewTarget != TargetActor.Get();

	if (UHealthComponent* OldHealth = TargetHealth.Get())
	{
		OldHealth->OnDeath.RemoveDynamic(this, &AEnemyAIController::HandleTargetDeath);
	}

	TargetActor = NewTarget;
	TargetHealth = nullptr;
	bTargetDead = false;

	if (NewTarget == nullptr)
	{
		return;
	}

	if (UHealthComponent* Health = NewTarget->FindComponentByClass<UHealthComponent>())
	{
		TargetHealth = Health;

		// A health component that has not begun play still holds 0 health, which would read as
		// dead. Only trust IsDead() once it started; otherwise the OnDeath event will tell us.
		bTargetDead = Health->HasBegunPlay() && Health->IsDead();

		if (!bTargetDead)
		{
			Health->OnDeath.AddDynamic(this, &AEnemyAIController::HandleTargetDeath);
		}
	}

	// The StateTree chase task starts its move once, so it would keep following the old target.
	// A new move request replaces the old one. It is skipped while attacking or stunned: those
	// states decide by themselves when to move again.
	if (bRepathOnTargetChange && bSwitchedToOtherTarget && !bTargetDead && GetPawn() != nullptr && !IsAttacking() && !IsStunned())
	{
		MoveToTarget();
	}
}

void AEnemyAIController::RefreshTarget()
{
	// A target that died, was destroyed or got farther than the give-up distance is dropped.
	// IsTargetInDeaggroRange is false for all of those cases.
	if (TargetActor != nullptr && !IsTargetInDeaggroRange())
	{
		SetTarget(nullptr);
	}

	// With no target the enemy looks around. The search only runs while it has none.
	if (TargetActor == nullptr)
	{
		SetTarget(FindNewTarget());
		return;
	}

	const APawn* ControlledPawn = GetPawn();
	if (ControlledPawn == nullptr)
	{
		return;
	}

	const FVector Location = ControlledPawn->GetActorLocation();

	// The decision is made on a local variable and applied once at the end, so the target
	// never flips back and forth inside one update.
	AActor* Chosen = TargetActor.Get();

	// The threat table (and a taunt) can pull the enemy to someone else.
	// Enemies without a threat component keep their target until they lose it.
	UThreatComponent* Threat = PawnThreat.Get();
	if (Threat != nullptr)
	{
		if (AActor* Preferred = Threat->PickTarget(Location, GetGiveUpRange(), Chosen))
		{
			Chosen = Preferred;
		}
	}

	// An opportunistic enemy that cannot reach its target hits whoever is within its reach.
	// A taunt forces the target for a while, so the reach rule is skipped meanwhile.
	// The search only runs while the chosen target is out of reach.
	const bool bTaunted = Threat != nullptr && Threat->HasForcedTarget();
	if (bOpportunistic && !bTaunted && FVector::Dist2D(Location, Chosen->GetActorLocation()) > AttackRange)
	{
		if (AActor* InReach = FindNearestHostile(AttackRange))
		{
			Chosen = InReach;
		}
	}

	if (Chosen != TargetActor.Get())
	{
		SetTarget(Chosen);
	}
}

void AEnemyAIController::UpdatePerception()
{
	RefreshTarget();

	const float Distance = GetDistanceToTarget();
	bTargetInAggroRange = Distance >= 0.f && Distance <= AggroRange;
	bTargetInAttackRange = Distance >= 0.f && Distance <= AttackRange;

	// The combat starts when the target is noticed and ends when it is lost. Between the aggro
	// and the deaggro distance the state is kept as it is, which avoids flickering.
	if (UEnemyInfoComponent* Info = PawnInfo.Get())
	{
		const bool bTargetInDeaggroRange = Distance >= 0.f && Distance <= GetGiveUpRange();

		if (bTargetInAggroRange)
		{
			Info->SetInCombat(true);
		}
		else if (Info->IsInCombat() && !bTargetInDeaggroRange)
		{
			Info->SetInCombat(false);

			// The fight is over: the next one starts without the old threat.
			if (UThreatComponent* ThreatTable = PawnThreat.Get())
			{
				ThreatTable->ClearThreat();
			}
		}
	}
}

void AEnemyAIController::HandleStunStarted(float Duration)
{
	bPawnStunned = true;

	// A stunned enemy stops walking right away.
	StopMovement();
}

void AEnemyAIController::HandleStunEnded()
{
	bPawnStunned = false;
}

void AEnemyAIController::HandleDamageTaken(float Amount, AActor* DamageCauser)
{
	UEnemyInfoComponent* Info = PawnInfo.Get();
	const UHealthComponent* Health = PawnHealth.Get();
	if (Info == nullptr || Health == nullptr)
	{
		return;
	}

	// A killing blow does not start a combat, the death handler closes it right after.
	// Only a hostile pawn starts the combat by hitting the enemy.
	if (Health->IsDead() || DamageCauser == nullptr || !UTeamComponent::AreHostile(GetPawn(), DamageCauser))
	{
		return;
	}

	// Hit by someone it had not noticed (for example from outside the aggro range):
	// the attacker becomes the target. An enemy already fighting keeps its target.
	if (ResolveTarget() == nullptr)
	{
		SetTarget(DamageCauser);
	}

	if (DamageCauser == ResolveTarget())
	{
		Info->SetInCombat(true);
	}
}

void AEnemyAIController::HandleDeath(AActor* Killer)
{
	// No more perception updates and no more decisions once the pawn is dead.
	GetWorldTimerManager().ClearTimer(PerceptionTimer);
	bTargetInAggroRange = false;
	bTargetInAttackRange = false;

	if (UEnemyInfoComponent* Info = PawnInfo.Get())
	{
		Info->SetInCombat(false);
	}

	StopMovement();

	// The StateTree component is a brain component, so this stops the running tree and its tasks.
	if (UBrainComponent* Brain = FindComponentByClass<UBrainComponent>())
	{
		Brain->StopLogic(TEXT("Pawn died"));
	}
}

void AEnemyAIController::HandleTargetDeath(AActor* Killer)
{
	bTargetDead = true;
	StopMovement();

	// Refresh right away: the dead target is dropped and the next one (by threat or by distance)
	// is picked. With nobody left the flags drop and the combat ends without waiting for the next
	// timer tick, and the StateTree leaves Chase and Attack because the conditions no longer hold.
	UpdatePerception();
}

void AEnemyAIController::ResetState()
{
	// Forget the fight: no movement, no target (the perception finds one again), no threat, out of combat.
	StopMovement();
	SetTarget(nullptr);
	bTargetInAggroRange = false;
	bTargetInAttackRange = false;

	if (UThreatComponent* Threat = PawnThreat.Get())
	{
		Threat->ClearThreat();
	}

	if (const UStunComponent* Stun = PawnStun.Get())
	{
		bPawnStunned = Stun->IsStunned();
	}

	if (UEnemyInfoComponent* Info = PawnInfo.Get())
	{
		Info->SetInCombat(false);
	}

	// The perception stops when the pawn dies, so it starts again here.
	GetWorldTimerManager().SetTimer(PerceptionTimer, this, &AEnemyAIController::UpdatePerception, PerceptionInterval, true);

	RestartDecisionLogic();
}

void AEnemyAIController::RestartDecisionLogic()
{
	// The StateTree component is a brain component. It was stopped when the pawn died.
	if (UBrainComponent* Brain = FindComponentByClass<UBrainComponent>())
	{
		Brain->RestartLogic();
	}
}

AActor* AEnemyAIController::GetTargetActor()
{
	return ResolveTarget();
}

float AEnemyAIController::GetDistanceToTarget()
{
	const APawn* ControlledPawn = GetPawn();
	const AActor* Target = ResolveTarget();
	if (ControlledPawn == nullptr || Target == nullptr)
	{
		return -1.f;
	}

	return FVector::Dist2D(ControlledPawn->GetActorLocation(), Target->GetActorLocation());
}

bool AEnemyAIController::IsTargetInAggroRange()
{
	const float Distance = GetDistanceToTarget();
	return Distance >= 0.f && Distance <= AggroRange;
}

bool AEnemyAIController::IsTargetInDeaggroRange()
{
	const float Distance = GetDistanceToTarget();

	// The give-up distance can never be smaller than the aggro distance.
	return Distance >= 0.f && Distance <= GetGiveUpRange();
}

bool AEnemyAIController::IsTargetInAttackRange()
{
	const float Distance = GetDistanceToTarget();
	return Distance >= 0.f && Distance <= AttackRange;
}

bool AEnemyAIController::MoveToTarget()
{
	AActor* Target = ResolveTarget();
	if (Target == nullptr)
	{
		return false;
	}

	// Stop at a comfortable melee distance. bStopOnOverlap is false so the capsule
	// radii are not added to the acceptance radius (it would stop outside the attack range).
	const float AcceptanceRadius = AttackRange * 0.7f;
	const EPathFollowingRequestResult::Type Result = MoveToActor(Target, AcceptanceRadius, false);
	return Result != EPathFollowingRequestResult::Failed;
}

void AEnemyAIController::FaceTarget()
{
	APawn* ControlledPawn = GetPawn();
	const AActor* Target = ResolveTarget();
	if (ControlledPawn == nullptr || Target == nullptr)
	{
		return;
	}

	FVector ToTarget = Target->GetActorLocation() - ControlledPawn->GetActorLocation();
	ToTarget.Z = 0.f;
	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	ControlledPawn->SetActorRotation(ToTarget.Rotation());
}

bool AEnemyAIController::RequestAttack()
{
	UMeleeAttackComponent* Melee = PawnMelee.Get();
	if (Melee == nullptr || Melee->IsAttacking())
	{
		return false;
	}

	FaceTarget();
	Melee->RequestAttack();
	return Melee->IsAttacking();
}

bool AEnemyAIController::IsAttacking() const
{
	const UMeleeAttackComponent* Melee = PawnMelee.Get();
	return Melee != nullptr && Melee->IsAttacking();
}

bool AEnemyAIController::IsStunned() const
{
	const UStunComponent* Stun = PawnStun.Get();
	return Stun != nullptr && Stun->IsStunned();
}