#include "AI/EnemyAIController.h"

#include "Combat/MeleeAttackComponent.h"
#include "Combat/StunComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

void AEnemyAIController::BeginPlay()
{
	// Resolve the target before the StateTree component starts (components begin inside Super).
	ResolveTarget();

	Super::BeginPlay();
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Cache the combat components of the controlled pawn once.
	if (InPawn != nullptr)
	{
		PawnMelee = InPawn->FindComponentByClass<UMeleeAttackComponent>();
		PawnStun = InPawn->FindComponentByClass<UStunComponent>();
	}

	// The player pawn may not exist yet at this point, so this can stay empty until BeginPlay.
	ResolveTarget();
}

void AEnemyAIController::OnUnPossess()
{
	PawnMelee = nullptr;
	PawnStun = nullptr;

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

bool AEnemyAIController::IsTargetInAttackRange()
{
	const float Distance = GetDistanceToTarget();
	return Distance >= 0.f && Distance <= AttackRange;
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