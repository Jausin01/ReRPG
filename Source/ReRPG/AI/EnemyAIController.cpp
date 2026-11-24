#include "AI/EnemyAIController.h"

#include "BrainComponent.h"
#include "Combat/HealthComponent.h"
#include "Combat/MeleeAttackComponent.h"
#include "Combat/StunComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "TimerManager.h"

void AEnemyAIController::BeginPlay()
{
	// Resolve the target and the flags before the StateTree component starts (components begin inside Super).
	ResolveTarget();
	UpdatePerception();

	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(PerceptionTimer, this, &AEnemyAIController::UpdatePerception, PerceptionInterval, true);
}

void AEnemyAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(PerceptionTimer);

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
	}

	if (UStunComponent* Stun = PawnStun.Get())
	{
		bPawnStunned = Stun->IsStunned();
		Stun->OnStunStarted.AddDynamic(this, &AEnemyAIController::HandleStunStarted);
		Stun->OnStunEnded.AddDynamic(this, &AEnemyAIController::HandleStunEnded);
	}

	if (UHealthComponent* Health = PawnHealth.Get())
	{
		Health->OnDeath.AddDynamic(this, &AEnemyAIController::HandleDeath);
	}

	// The player pawn may not exist yet at this point, so this can stay empty until BeginPlay.
	ResolveTarget();
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
		Health->OnDeath.RemoveDynamic(this, &AEnemyAIController::HandleDeath);
	}

	PawnMelee = nullptr;
	PawnStun = nullptr;
	PawnHealth = nullptr;
	bPawnStunned = false;

	Super::OnUnPossess();
}

AActor* AEnemyAIController::ResolveTarget()
{
	if (!IsValid(TargetActor))
	{
		TargetActor = UGameplayStatics::GetPlayerPawn(this, 0);
	}

	return TargetActor;
}

void AEnemyAIController::UpdatePerception()
{
	const float Distance = GetDistanceToTarget();
	bTargetInAggroRange = Distance >= 0.f && Distance <= AggroRange;
	bTargetInAttackRange = Distance >= 0.f && Distance <= AttackRange;
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

void AEnemyAIController::HandleDeath(AActor* Killer)
{
	// No more perception updates and no more decisions once the pawn is dead.
	GetWorldTimerManager().ClearTimer(PerceptionTimer);
	bTargetInAggroRange = false;
	bTargetInAttackRange = false;

	StopMovement();

	// The StateTree component is a brain component, so this stops the running tree and its tasks.
	if (UBrainComponent* Brain = FindComponentByClass<UBrainComponent>())
	{
		Brain->StopLogic(TEXT("Pawn died"));
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
	return Distance >= 0.f && Distance <= FMath::Max(DeaggroRange, AggroRange);
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