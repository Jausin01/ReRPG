#include "AI/BTService_UpdateBossState.h"

#include "AI/BossData.h"
#include "AI/BossPhaseComponent.h"
#include "AI/EnemyAIController.h"
#include "AI/EnemyInfoComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "GameFramework/Pawn.h"

UBTService_UpdateBossState::UBTService_UpdateBossState()
{
	NodeName = TEXT("Update Boss State");
	Interval = 0.1f;
	RandomDeviation = 0.f;

	// Restrict the key pickers in the editor to keys of the right type.
	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateBossState, TargetKey), AActor::StaticClass());
	InCombatKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateBossState, InCombatKey));
	InAttackRangeKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateBossState, InAttackRangeKey));
	TransitioningKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateBossState, TransitioningKey));
	AppliedPhaseKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateBossState, AppliedPhaseKey));
}

void UBTService_UpdateBossState::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	// Turns the key names chosen in the editor into keys of this blackboard.
	if (const UBlackboardData* BlackboardAsset = GetBlackboardAsset())
	{
		TargetKey.ResolveSelectedKey(*BlackboardAsset);
		InCombatKey.ResolveSelectedKey(*BlackboardAsset);
		InAttackRangeKey.ResolveSelectedKey(*BlackboardAsset);
		TransitioningKey.ResolveSelectedKey(*BlackboardAsset);
		AppliedPhaseKey.ResolveSelectedKey(*BlackboardAsset);
	}
}

void UBTService_UpdateBossState::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	// No pattern injected yet: the first tick applies the one of the current phase.
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (Blackboard != nullptr && AppliedPhaseKey.IsSet())
	{
		Blackboard->SetValueAsInt(AppliedPhaseKey.SelectedKeyName, INDEX_NONE);
	}
}

void UBTService_UpdateBossState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AEnemyAIController* Controller = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
	const APawn* Pawn = Controller != nullptr ? Controller->GetPawn() : nullptr;
	if (Blackboard == nullptr || Pawn == nullptr)
	{
		return;
	}

	const UEnemyInfoComponent* Info = Pawn->FindComponentByClass<UEnemyInfoComponent>();
	const UBossPhaseComponent* Phase = Pawn->FindComponentByClass<UBossPhaseComponent>();

	if (TargetKey.IsSet())
	{
		Blackboard->SetValueAsObject(TargetKey.SelectedKeyName, Controller->GetTargetActor());
	}

	if (InCombatKey.IsSet())
	{
		Blackboard->SetValueAsBool(InCombatKey.SelectedKeyName, Info != nullptr && Info->IsInCombat());
	}

	if (InAttackRangeKey.IsSet())
	{
		Blackboard->SetValueAsBool(InAttackRangeKey.SelectedKeyName, Controller->IsTargetInAttackRange());
	}

	if (TransitioningKey.IsSet())
	{
		Blackboard->SetValueAsBool(TransitioningKey.SelectedKeyName, Phase != nullptr && Phase->IsTransitioning());
	}

	if (Phase != nullptr)
	{
		ApplyPhasePattern(OwnerComp, *Blackboard, *Phase);
	}
}

void UBTService_UpdateBossState::ApplyPhasePattern(UBehaviorTreeComponent& OwnerComp, UBlackboardComponent& Blackboard, const UBossPhaseComponent& Phase) const
{
	if (!AppliedPhaseKey.IsSet() || !PatternTag.IsValid())
	{
		return;
	}

	// The phase component may not have started yet (no phase), or the pattern is already injected.
	const int32 PhaseIndex = Phase.GetPhaseIndex();
	if (PhaseIndex == INDEX_NONE || PhaseIndex == Blackboard.GetValueAsInt(AppliedPhaseKey.SelectedKeyName))
	{
		return;
	}

	// A phase without a pattern keeps the previous one.
	const FBossPhase* Data = Phase.GetCurrentPhase();
	if (Data != nullptr && Data->AttackPattern != nullptr)
	{
		OwnerComp.SetDynamicSubtree(PatternTag, Data->AttackPattern.Get());
	}

	Blackboard.SetValueAsInt(AppliedPhaseKey.SelectedKeyName, PhaseIndex);
}