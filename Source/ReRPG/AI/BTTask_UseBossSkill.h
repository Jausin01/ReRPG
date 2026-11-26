#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_UseBossSkill.generated.h"

// Faces the target, starts a skill of the current phase and stays running until the skill
// is released or interrupted. Fails if no skill could start (cooldown, stun, no skills).
UCLASS()
class RERPG_API UBTTask_UseBossSkill : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_UseBossSkill();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};