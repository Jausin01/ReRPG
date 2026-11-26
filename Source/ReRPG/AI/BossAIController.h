#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyAIController.h"
#include "BossAIController.generated.h"

class UBehaviorTree;
class UBossPhaseComponent;
class UHealthComponent;

// Controller of bosses. It inherits everything of the enemy controller (target, perception,
// combat state, stopping when the target dies) and adds a Behavior Tree and the choice of
// the target by phase (Focus). The attack pattern of each phase is injected into the tree
// by the UpdateBossState service.
UCLASS()
class RERPG_API ABossAIController : public AEnemyAIController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	// Used when the boss has no target or its target died: follows the focus of the phase.
	virtual AActor* FindNewTarget() override;

private:
	// Best hostile pawn for the focus of the current phase, or null if the focus is Default
	// or nobody is in range.
	AActor* PickFocusTarget() const;

	// Switches to the focus target when it is not the current one.
	void UpdateFocus();

	UFUNCTION()
	void HandlePhaseChanged(int32 PhaseIndex);

	UFUNCTION()
	void HandleBossDeath(AActor* Killer);

	// Main tree of the boss (blackboard, transition handling and the phase pattern slot).
	UPROPERTY(EditDefaultsOnly, Category = "Boss AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	// Time between two focus checks. Keeps the boss from changing its mind every instant.
	UPROPERTY(EditDefaultsOnly, Category = "Boss AI", meta = (ClampMin = "0.2"))
	float FocusInterval = 2.f;

	FTimerHandle FocusTimer;
	TWeakObjectPtr<UBossPhaseComponent> BossPhase;
	TWeakObjectPtr<UHealthComponent> BossHealth;
};