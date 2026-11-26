#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyAIController.h"
#include "BossAIController.generated.h"

class UBehaviorTree;

// Controller of bosses. It inherits everything of the enemy controller (target, perception,
// combat state, stopping when the target dies) and only adds a Behavior Tree. The attack
// pattern of each phase is injected into that tree by the UpdateBossState service.
UCLASS()
class RERPG_API ABossAIController : public AEnemyAIController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	// Main tree of the boss (blackboard, transition handling and the phase pattern slot).
	UPROPERTY(EditDefaultsOnly, Category = "Boss AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;
};