#include "AI/BTService_UpdateAllyState.h"

#include "AI/AllyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"

UBTService_UpdateAllyState::UBTService_UpdateAllyState()
{
	NodeName = TEXT("Update Ally State");
	Interval = 0.2f;
	RandomDeviation = 0.f;

	// Restrict the key pickers in the editor to keys of the right type.
	MoveGoalKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateAllyState, MoveGoalKey));
	WantsToMoveKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateAllyState, WantsToMoveKey));
	WantsToAttackKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateAllyState, WantsToAttackKey));
}

void UBTService_UpdateAllyState::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	// Turns the key names chosen in the editor into keys of this blackboard.
	if (const UBlackboardData* BlackboardAsset = GetBlackboardAsset())
	{
		MoveGoalKey.ResolveSelectedKey(*BlackboardAsset);
		WantsToMoveKey.ResolveSelectedKey(*BlackboardAsset);
		WantsToAttackKey.ResolveSelectedKey(*BlackboardAsset);
	}
}

void UBTService_UpdateAllyState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AAllyAIController* Controller = Cast<AAllyAIController>(OwnerComp.GetAIOwner());
	if (Blackboard == nullptr || Controller == nullptr)
	{
		return;
	}

	const FAllyPlan Plan = Controller->MakePlan();

	// The goal is only written while the ally wants to move, so the node that is walking
	// never sees the goal jump to zero when the ally arrives.
	if (Plan.bWantsToMove && MoveGoalKey.IsSet())
	{
		Blackboard->SetValueAsVector(MoveGoalKey.SelectedKeyName, Plan.MoveGoal);
	}

	if (WantsToMoveKey.IsSet())
	{
		Blackboard->SetValueAsBool(WantsToMoveKey.SelectedKeyName, Plan.bWantsToMove);
	}

	if (WantsToAttackKey.IsSet())
	{
		Blackboard->SetValueAsBool(WantsToAttackKey.SelectedKeyName, Plan.bWantsToAttack);
	}
}