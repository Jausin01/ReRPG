#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTService_UpdateAllyState.generated.h"

// Keeps the blackboard of an ally up to date with what the ally controller wants to do.
// Place it on the root selector of the ally tree.
UCLASS()
class RERPG_API UBTService_UpdateAllyState : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateAllyState();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	// Vector key: where the ally wants to go.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector MoveGoalKey;

	// Bool key: true while the ally is not where it wants to be.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector WantsToMoveKey;

	// Bool key: true while the ally can hit its target.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector WantsToAttackKey;
};