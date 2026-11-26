#include "AI/BossAIController.h"

#include "AI/BossData.h"
#include "AI/BossPhaseComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Combat/CombatTargeting.h"
#include "Combat/HealthComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

void ABossAIController::BeginPlay()
{
	Super::BeginPlay();

	// The focus is checked on a timer, not every frame.
	GetWorldTimerManager().SetTimer(FocusTimer, this, &ABossAIController::UpdateFocus, FocusInterval, true);

	if (BehaviorTree == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no Behavior Tree set."), *GetName());
		return;
	}

	// Creates the blackboard from the tree's asset and starts the tree.
	RunBehaviorTree(BehaviorTree);
}

void ABossAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(FocusTimer);

	Super::EndPlay(EndPlayReason);
}

void ABossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (InPawn == nullptr)
	{
		return;
	}

	// Cache the boss components once. A new phase can change the focus, and a dead boss stops choosing.
	BossPhase = InPawn->FindComponentByClass<UBossPhaseComponent>();
	BossHealth = InPawn->FindComponentByClass<UHealthComponent>();

	if (UBossPhaseComponent* Phase = BossPhase.Get())
	{
		Phase->OnPhaseChanged.AddDynamic(this, &ABossAIController::HandlePhaseChanged);
	}

	if (UHealthComponent* Health = BossHealth.Get())
	{
		Health->OnDeath.AddDynamic(this, &ABossAIController::HandleBossDeath);
	}
}

void ABossAIController::OnUnPossess()
{
	if (UBossPhaseComponent* Phase = BossPhase.Get())
	{
		Phase->OnPhaseChanged.RemoveDynamic(this, &ABossAIController::HandlePhaseChanged);
	}

	if (UHealthComponent* Health = BossHealth.Get())
	{
		Health->OnDeath.RemoveDynamic(this, &ABossAIController::HandleBossDeath);
	}

	BossPhase = nullptr;
	BossHealth = nullptr;

	Super::OnUnPossess();
}

AActor* ABossAIController::FindNewTarget()
{
	AActor* Focus = PickFocusTarget();
	return Focus != nullptr ? Focus : Super::FindNewTarget();
}

void ABossAIController::RestartDecisionLogic()
{
	// The Behavior Tree was started from code, so running it again restarts it from the root.
	// The pattern of the first phase is injected again by the service.
	if (BehaviorTree != nullptr)
	{
		RunBehaviorTree(BehaviorTree);
	}

	// The focus timer stops when the boss dies.
	GetWorldTimerManager().SetTimer(FocusTimer, this, &ABossAIController::UpdateFocus, FocusInterval, true);
}

AActor* ABossAIController::PickFocusTarget() const
{
	const APawn* BossPawn = GetPawn();
	const UBossPhaseComponent* Phase = BossPhase.Get();
	const FBossPhase* PhaseData = Phase != nullptr ? Phase->GetCurrentPhase() : nullptr;

	// Default keeps the current target, so there is nothing to pick.
	if (BossPawn == nullptr || PhaseData == nullptr || PhaseData->Focus == EBossFocus::Default)
	{
		return nullptr;
	}

	const FVector BossLocation = BossPawn->GetActorLocation();

	TArray<TWeakObjectPtr<UHealthComponent>> Candidates;
	CombatTargeting::FindHostileTargets(BossPawn, BossLocation, GetAggroRange(), Candidates);

	// The lowest score wins: distance for Nearest, current health for LowestHealth.
	AActor* Best = nullptr;
	float BestScore = TNumericLimits<float>::Max();
	for (const TWeakObjectPtr<UHealthComponent>& Candidate : Candidates)
	{
		const UHealthComponent* Health = Candidate.Get();
		AActor* Actor = Health != nullptr ? Health->GetOwner() : nullptr;
		if (Actor == nullptr || Health->IsDead())
		{
			continue;
		}

		const float Score = PhaseData->Focus == EBossFocus::Nearest
			? FVector::Dist2D(BossLocation, Actor->GetActorLocation())
			: Health->GetHealth();

		if (Score < BestScore)
		{
			BestScore = Score;
			Best = Actor;
		}
	}

	return Best;
}

void ABossAIController::UpdateFocus()
{
	AActor* Focus = PickFocusTarget();
	if (Focus != nullptr && Focus != GetTargetActor())
	{
		SetTarget(Focus);
	}
}

void ABossAIController::HandlePhaseChanged(int32 PhaseIndex)
{
	// The new phase may have another focus: apply it now instead of waiting for the timer.
	UpdateFocus();
}

void ABossAIController::HandleBossDeath(AActor* Killer)
{
	GetWorldTimerManager().ClearTimer(FocusTimer);
}