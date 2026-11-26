#include "AI/BTTask_UseBossSkill.h"

#include "AI/BossSkillComponent.h"
#include "AI/EnemyAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"

UBTTask_UseBossSkill::UBTTask_UseBossSkill()
{
	NodeName = TEXT("Use Boss Skill");

	// The wind-up has no "finished" call into the tree, so the task checks it while it runs.
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_UseBossSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyAIController* Controller = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
	const APawn* Pawn = Controller != nullptr ? Controller->GetPawn() : nullptr;
	UBossSkillComponent* Skills = Pawn != nullptr ? Pawn->FindComponentByClass<UBossSkillComponent>() : nullptr;

	// Never starts a skill in the middle of a basic attack.
	if (Skills == nullptr || Controller->IsAttacking())
	{
		return EBTNodeResult::Failed;
	}

	// Skills hit in front of the boss, so it turns before the wind-up (the effects may run at once).
	Controller->FaceTarget();

	if (!Skills->TryUseSkill())
	{
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_UseBossSkill::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	const AAIController* Controller = OwnerComp.GetAIOwner();
	const APawn* Pawn = Controller != nullptr ? Controller->GetPawn() : nullptr;
	const UBossSkillComponent* Skills = Pawn != nullptr ? Pawn->FindComponentByClass<UBossSkillComponent>() : nullptr;

	if (Skills == nullptr || !Skills->IsCasting())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}