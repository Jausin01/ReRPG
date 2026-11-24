#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

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

	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	bool IsTargetInAttackRange();

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
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:
	// Fills TargetActor if it is empty or no longer valid.
	AActor* ResolveTarget();

	// Distance at which the enemy notices the target.
	UPROPERTY(EditDefaultsOnly, Category = "Enemy AI", meta = (ClampMin = "0.0"))
	float AggroRange = 1500.f;

	// Distance at which the enemy starts attacking (keep it inside the attack step range + radius).
	UPROPERTY(EditDefaultsOnly, Category = "Enemy AI", meta = (ClampMin = "0.0"))
	float AttackRange = 150.f;

	// Readable and bindable from the StateTree.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> TargetActor;

	TWeakObjectPtr<UMeleeAttackComponent> PawnMelee;
	TWeakObjectPtr<UStunComponent> PawnStun;
};