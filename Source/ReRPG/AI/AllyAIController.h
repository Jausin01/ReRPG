#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyAIController.h"
#include "AllyAIController.generated.h"

class UBehaviorTree;
class UPartyMemberComponent;
class UPartyRoleData;

// What an ally wants to do right now. The Behavior Tree service copies it to the blackboard.
struct FAllyPlan
{
	// Where to go. Only meaningful while bWantsToMove is true.
	FVector MoveGoal = FVector::ZeroVector;

	bool bWantsToMove = false;
	bool bWantsToAttack = false;
};

// Controller of party members. It inherits everything of the enemy controller (target, perception,
// attack, death, reset) and adds a Behavior Tree, a target chosen by the role and the positioning
// of the role (tank close, mage far, everybody follows the player when there is no fight).
UCLASS()
class RERPG_API AAllyAIController : public AEnemyAIController
{
	GENERATED_BODY()

public:
	// Decides what the ally should do right now. Called by the Behavior Tree service on a timer.
	FAllyPlan MakePlan();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	// The target follows the rule of the role, among the enemies near the player.
	virtual AActor* FindNewTarget() override;

	// After a reset the Behavior Tree runs again (it stops when the pawn dies).
	virtual void RestartDecisionLogic() override;

private:
	// Role of the controlled pawn, or null if it has none (the ally then uses default values).
	const UPartyRoleData* GetRole() const;

	// Main tree of the ally (blackboard with MoveGoal, WantsToMove and WantsToAttack).
	UPROPERTY(EditDefaultsOnly, Category = "Ally AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	TWeakObjectPtr<UPartyMemberComponent> PartyMember;
};