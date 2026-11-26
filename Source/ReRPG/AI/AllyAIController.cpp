#include "AI/AllyAIController.h"

#include "AI/PartyMemberComponent.h"
#include "AI/PartyRoleData.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Combat/CombatTargeting.h"
#include "Combat/HealthComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	// Used by an ally that has no role assigned: a plain melee fighter.
	constexpr float DefaultMaxRange = 130.f;
	constexpr float DefaultFollowDistance = 350.f;

	// A point at the given distance from Center, on the side where From is. Moving there keeps
	// the ally on its own side of the target instead of walking through it.
	FVector PointAround(const FVector& Center, const FVector& From, float Distance)
	{
		FVector Away = From - Center;
		Away.Z = 0.f;
		Away = Away.GetSafeNormal();

		// Standing exactly on the center: any direction works.
		if (Away.IsNearlyZero())
		{
			Away = FVector::ForwardVector;
		}

		return Center + Away * Distance;
	}
}

void AAllyAIController::BeginPlay()
{
	Super::BeginPlay();

	if (BehaviorTree == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no Behavior Tree set."), *GetName());
		return;
	}

	// Creates the blackboard from the tree's asset and starts the tree.
	RunBehaviorTree(BehaviorTree);
}

void AAllyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (InPawn != nullptr)
	{
		PartyMember = InPawn->FindComponentByClass<UPartyMemberComponent>();
	}
}

void AAllyAIController::OnUnPossess()
{
	PartyMember = nullptr;

	Super::OnUnPossess();
}

const UPartyRoleData* AAllyAIController::GetRole() const
{
	const UPartyMemberComponent* Member = PartyMember.Get();
	return Member != nullptr ? Member->GetRole() : nullptr;
}

AActor* AAllyAIController::FindNewTarget()
{
	const APawn* Self = GetPawn();
	if (Self == nullptr)
	{
		return nullptr;
	}

	// The party fights what is near the player, so an ally that fell behind still helps.
	const APawn* Leader = UGameplayStatics::GetPlayerPawn(this, 0);
	const FVector SelfLocation = Self->GetActorLocation();
	const FVector SearchCenter = Leader != nullptr ? Leader->GetActorLocation() : SelfLocation;

	TArray<TWeakObjectPtr<UHealthComponent>> Candidates;
	CombatTargeting::FindHostileTargets(Self, SearchCenter, GetAggroRange(), Candidates);

	const UPartyRoleData* PartyRole = GetRole();
	const bool bLowestHealth = PartyRole != nullptr && PartyRole->GetTargetRule() == EPartyTargetRule::LowestHealth;

	// The lowest score wins: current health or distance to the ally, by the rule of the role.
	AActor* Best = nullptr;
	float BestScore = TNumericLimits<float>::Max();
	for (const TWeakObjectPtr<UHealthComponent>& Candidate : Candidates)
	{
		const UHealthComponent* Health = Candidate.Get();
		AActor* Actor = Health != nullptr ? Health->GetOwner() : nullptr;

		// A health component that has not begun play still holds 0 health: do not read it as dead.
		if (Actor == nullptr || (Health->HasBegunPlay() && Health->IsDead()))
		{
			continue;
		}

		const float Score = bLowestHealth ? Health->GetHealth() : FVector::Dist2D(SelfLocation, Actor->GetActorLocation());
		if (Score < BestScore)
		{
			BestScore = Score;
			Best = Actor;
		}
	}

	return Best;
}

FAllyPlan AAllyAIController::MakePlan()
{
	FAllyPlan Plan;

	const APawn* Self = GetPawn();
	if (Self == nullptr)
	{
		return Plan;
	}

	const UPartyRoleData* PartyRole = GetRole();
	const float MinRange = PartyRole != nullptr ? PartyRole->GetMinRange() : 0.f;
	const float MaxRange = PartyRole != nullptr ? PartyRole->GetMaxRange() : DefaultMaxRange;
	const float FollowDistance = PartyRole != nullptr ? PartyRole->GetFollowDistance() : DefaultFollowDistance;
	const bool bCanMelee = PartyRole == nullptr || PartyRole->CanMelee();

	const FVector SelfLocation = Self->GetActorLocation();

	// In a fight: keep the distance of the role. Closer than the minimum the ally backs away,
	// farther than the maximum it approaches, and inside the range it holds its position.
	if (const AActor* Target = GetTargetActor())
	{
		const FVector TargetLocation = Target->GetActorLocation();
		const float Distance = FVector::Dist2D(SelfLocation, TargetLocation);

		if (Distance < MinRange || Distance > MaxRange)
		{
			Plan.MoveGoal = PointAround(TargetLocation, SelfLocation, (MinRange + MaxRange) * 0.5f);
			Plan.bWantsToMove = true;
		}

		Plan.bWantsToAttack = bCanMelee && IsTargetInAttackRange();
		return Plan;
	}

	// No fight: stay close to the player.
	if (const APawn* Leader = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		const FVector LeaderLocation = Leader->GetActorLocation();
		if (FVector::Dist2D(SelfLocation, LeaderLocation) > FollowDistance)
		{
			Plan.MoveGoal = PointAround(LeaderLocation, SelfLocation, FollowDistance * 0.6f);
			Plan.bWantsToMove = true;
		}
	}

	return Plan;
}

void AAllyAIController::RestartDecisionLogic()
{
	// The Behavior Tree was started from code, so running it again restarts it from the root.
	if (BehaviorTree != nullptr)
	{
		RunBehaviorTree(BehaviorTree);
	}
}