#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockOnComponent.generated.h"

class ACharacter;
class UHealthComponent;

// NewTarget is null when the lock is released.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLockOnTargetChanged, AActor*, NewTarget);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class RERPG_API ULockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULockOnComponent();

	// Locks onto the best target in front of the camera, or releases the current lock.
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void ToggleLockOn();

	// Switches to the nearest target on one side of the current one.
	// Direction: negative = left, positive = right (relative to the camera).
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void SwitchTarget(float Direction);

	UFUNCTION(BlueprintPure, Category = "LockOn")
	bool IsLockedOn() const { return bIsLockedOn; }

	UFUNCTION(BlueprintPure, Category = "LockOn")
	AActor* GetTarget() const { return LockedTarget.Get(); }

	// Reserved for the lock-on reticle in the UI.
	UPROPERTY(BlueprintAssignable, Category = "LockOn")
	FOnLockOnTargetChanged OnTargetChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void SetTarget(AActor* NewTarget);
	void BeginLock();
	void EndLock();

	AActor* FindBestTarget() const;
	void GatherCandidates(TArray<AActor*>& OutCandidates) const;
	bool HasLineOfSight(const AActor* Target) const;

	// Runs on a timer (not every frame) to drop the lock when it stops making sense.
	void ValidateTarget();

	UFUNCTION()
	void HandleTargetDeath(AActor* Killer);

	// Maximum distance to acquire or switch to a target.
	UPROPERTY(EditDefaultsOnly, Category = "LockOn", meta = (ClampMin = "100.0"))
	float MaxRange = 1500.f;

	// Maximum angle from the camera center to pick the initial target.
	UPROPERTY(EditDefaultsOnly, Category = "LockOn", meta = (ClampMin = "1.0", ClampMax = "180.0"))
	float MaxAcquireAngle = 60.f;

	// The lock only breaks past MaxRange * this value, so it does not flicker at the edge.
	UPROPERTY(EditDefaultsOnly, Category = "LockOn", meta = (ClampMin = "1.0"))
	float BreakRangeMultiplier = 1.2f;

	// Minimum time between two target switches (protects against free-spinning wheels).
	UPROPERTY(EditDefaultsOnly, Category = "LockOn", meta = (ClampMin = "0.0"))
	float SwitchCooldown = 0.2f;

	// Time between lock validity checks (range, line of sight, death).
	UPROPERTY(EditDefaultsOnly, Category = "LockOn", meta = (ClampMin = "0.05"))
	float ValidationInterval = 0.25f;

	// How fast the camera turns toward the target.
	UPROPERTY(EditDefaultsOnly, Category = "LockOn|Camera", meta = (ClampMin = "0.1"))
	float CameraInterpSpeed = 8.f;

	// Fixed camera pitch while locked (negative = looking slightly down).
	UPROPERTY(EditDefaultsOnly, Category = "LockOn|Camera")
	float LockedPitch = -25.f;

	// Height above the actor origin used for the line of sight trace.
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	float LineOfSightHeight = 50.f;

	FTimerHandle ValidateTimer;
	TWeakObjectPtr<ACharacter> OwnerCharacter;
	TWeakObjectPtr<AActor> LockedTarget;
	TWeakObjectPtr<UHealthComponent> TargetHealth;
	float NextSwitchTime = 0.f;
	bool bIsLockedOn = false;

	// Character settings saved when the lock starts, restored when it ends.
	bool bPrevOrientToMovement = true;
	bool bPrevUseControllerYaw = false;
};