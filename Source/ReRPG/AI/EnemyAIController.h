#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class UEnemyInfoComponent;
class UHealthComponent;
class UMeleeAttackComponent;
class UStunComponent;
class UThreatComponent;

// Base controller for combat enemies. It only exposes simple queries and actions on top of
// the combat components; the decision making lives in the StateTree asset.
UCLASS()
class RERPG_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	// Returns the current target: the hostile pawn this enemy is fighting.
	// Returns null while there is no target or the target is dead.
	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	AActor* GetTargetActor();

	// Distance to the target on the ground plane, or -1 if there is no target or pawn.
	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	float GetDistanceToTarget();

	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	bool IsTargetInAggroRange();

	// True while the target is closer than the give-up distance (always >= AggroRange).
	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	bool IsTargetInDeaggroRange();

	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	bool IsTargetInAttackRange();

	// Starts moving toward the target. Returns false if no path request could be made.
	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	bool MoveToTarget();

	// Turns the controlled pawn toward the target (instant snap for now).
	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	void FaceTarget();

	// Faces the target and starts an attack. Returns true if an attack started.
	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	bool RequestAttack();

	UFUNCTION(BlueprintPure, Category = "Enemy AI")
	bool IsAttacking() const;

	UFUNCTION(BlueprintPure, Category = "Enemy AI")
	bool IsStunned() const;

	// Puts the AI back as if the fight never happened: no target, no threat, out of combat,
	// perception and decision logic running again (they stop when the pawn dies).
	void ResetState();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	// Picks a new target when there is none: the source with the most threat if the pawn has a
	// threat component and someone hurt it, otherwise the nearest living hostile pawn inside the
	// aggro range, or null. Smarter enemies and bosses override it to choose by their own rules.
	virtual AActor* FindNewTarget();

	// Starts the decision logic again after a reset. Enemies restart their StateTree;
	// subclasses with another kind of logic (the boss Behavior Tree) override this.
	virtual void RestartDecisionLogic();

	// Switches the target and listens to its death. Pass null to just release the old one.
	void SetTarget(AActor* NewTarget);

	float GetAggroRange() const { return AggroRange; }

private:
	// Current target, or null if there is none or it died. It never searches: the search runs
	// on the perception timer, because the StateTree asks for the target many times per tick.
	AActor* ResolveTarget() const;

	// Nearest living hostile pawn inside the radius around the controlled pawn, or null.
	AActor* FindNearestHostile(float Radius) const;

	// Drops a target that died or got too far, looks for a new one if there is none, then decides
	// if the current target stays: the threat table can pull the enemy to someone with clearly
	// more threat, and an opportunistic enemy turns on someone that is within its reach.
	void RefreshTarget();

	// Refreshes the target, the range flags and the combat state. Runs on a timer, not every frame.
	void UpdatePerception();

	UFUNCTION()
	void HandleStunStarted(float Duration);

	UFUNCTION()
	void HandleStunEnded();

	// Being hit by a hostile pawn starts the combat, even from outside the aggro range.
	UFUNCTION()
	void HandleDamageTaken(float Amount, AActor* DamageCauser);

	// Stops movement and the decision logic when the controlled pawn dies.
	UFUNCTION()
	void HandleDeath(AActor* Killer);

	// The target died: the enemy looks for another one. If there is none it forgets the target,
	// stops moving and leaves the combat.
	UFUNCTION()
	void HandleTargetDeath(AActor* Killer);

	// Distance at which the enemy gives up chasing (never smaller than the aggro distance).
	float GetGiveUpRange() const { return FMath::Max(DeaggroRange, AggroRange); }

	// Distance at which the enemy notices a hostile pawn.
	UPROPERTY(EditDefaultsOnly, Category = "Enemy AI", meta = (ClampMin = "0.0"))
	float AggroRange = 1500.f;

	// Distance at which the enemy gives up chasing. Keep it larger than AggroRange to avoid flickering.
	UPROPERTY(EditDefaultsOnly, Category = "Enemy AI", meta = (ClampMin = "0.0"))
	float DeaggroRange = 3000.f;

	// Distance at which the enemy starts attacking (keep it inside the attack step range + radius).
	UPROPERTY(EditDefaultsOnly, Category = "Enemy AI", meta = (ClampMin = "0.0"))
	float AttackRange = 150.f;

	// Time between perception updates (target search and range checks).
	UPROPERTY(EditDefaultsOnly, Category = "Enemy AI", meta = (ClampMin = "0.05"))
	float PerceptionInterval = 0.25f;

	// A simple enemy hits whoever is within its reach: while it chases a target it cannot reach,
	// it turns on a hostile pawn that is inside its attack range (a tank running beside it).
	// Smarter enemies leave this off and keep going after their priority target.
	UPROPERTY(EditDefaultsOnly, Category = "Enemy AI|Intelligence")
	bool bOpportunistic = false;

	// Readable and bindable from the StateTree.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy AI", meta = (AllowPrivateAccess = "true"))
	bool bTargetInAggroRange = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy AI", meta = (AllowPrivateAccess = "true"))
	bool bTargetInAttackRange = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy AI", meta = (AllowPrivateAccess = "true"))
	bool bPawnStunned = false;

	// True once the current target has died. A dead target counts as no target.
	bool bTargetDead = false;

	FTimerHandle PerceptionTimer;
	TWeakObjectPtr<UMeleeAttackComponent> PawnMelee;
	TWeakObjectPtr<UStunComponent> PawnStun;
	TWeakObjectPtr<UHealthComponent> PawnHealth;
	TWeakObjectPtr<UEnemyInfoComponent> PawnInfo;
	TWeakObjectPtr<UThreatComponent> PawnThreat;
	TWeakObjectPtr<UHealthComponent> TargetHealth;
};