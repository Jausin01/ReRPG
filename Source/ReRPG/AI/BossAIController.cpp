#include "AI/BossAIController.h"

#include "BehaviorTree/BehaviorTree.h"

void ABossAIController::BeginPlay()
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