#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class UHealthComponent;
class UMeleeAttackComponent;
class UStunComponent;

// Base controller for combat enemies. It only exposes simple queries and actions on top of
// the combat components; the decision making lives in the StateTree asset.
UCLASS()
class RERPG_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	// Returns the current target (the player pawn), looking it up again if needed.
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

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:
	// Fills TargetActor if it is empty or no longer valid.
	AActor* ResolveTarget();

	// Refreshes the range flags. Runs on a timer, not every frame.
	void UpdatePerception();

	UFUNCTION()
	void HandleStunStarted(float Duration);

	UFUNCTION()
	void HandleStunEnded();

	// Stops movement and the decision logic when the controlled pawn dies.
	UFUNCTION()
	void HandleDeath(AActor* Killer);

	// Distance at which the enemy notices the target.
	UPROPERTY(EditDefaultsOnly, Category = "Enemy AI", meta = (ClampMin = "0.0"))
	float AggroRange = 1500.f;

	// Distance at which the enemy gives up chasing. Keep it larger than AggroRange to avoid flickering.
	UPROPERTY(EditDefaultsOnly, Category = "Enemy AI", meta = (ClampMin = "0.0"))
	float DeaggroRange = 3000.f;

	// Distance at which the enemy starts attacking (keep it inside the attack step range + radius).
	UPROPERTY(EditDefaultsOnly, Category = "Enemy AI", meta = (ClampMin = "0.0"))
	float AttackRange = 150.f;

	// Time between range checks.
	UPROPERTY(EditDefaultsOnly, Category = "Enemy AI", meta = (ClampMin = "0.05"))
	float PerceptionInterval = 0.25f;

	// Readable and bindable from the StateTree.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy AI", meta = (AllowPrivateAccess = "true"))
	bool bTargetInAggroRange = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy AI", meta = (AllowPrivateAccess = "true"))
	bool bTargetInAttackRange = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy AI", meta = (AllowPrivateAccess = "true"))
	bool bPawnStunned = false;

	FTimerHandle PerceptionTimer;
	TWeakObjectPtr<UMeleeAttackComponent> PawnMelee;
	TWeakObjectPtr<UStunComponent> PawnStun;
	TWeakObjectPtr<UHealthComponent> PawnHealth;
};