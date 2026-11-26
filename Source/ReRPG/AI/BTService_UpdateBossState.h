#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "GameplayTagContainer.h"
#include "BTService_UpdateBossState.generated.h"

class UBlackboardComponent;
class UBossPhaseComponent;

// Keeps the blackboard of a boss up to date and injects the attack pattern of the current phase
// into the Run Behavior Dynamic node that uses the same tag. Place it on the root selector.
UCLASS()
class RERPG_API UBTService_UpdateBossState : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateBossState();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	// Injects the pattern of the current phase when the phase changed since the last time.
	void ApplyPhasePattern(UBehaviorTreeComponent& OwnerComp, UBlackboardComponent& Blackboard, const UBossPhaseComponent& Phase) const;

	// Object key: the target of the boss.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	// Bool key: true while the boss is in combat.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector InCombatKey;

	// Bool key: true while the target is inside the attack range.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector InAttackRangeKey;

	// Bool key: true while the boss is changing phase.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TransitioningKey;

	// Int key: the last phase whose pattern was injected (keeps the service from injecting twice).
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector AppliedPhaseKey;

	// Must match the Injection Tag of the Run Behavior Dynamic node.
	UPROPERTY(EditAnywhere, Category = "Boss")
	FGameplayTag PatternTag;
};