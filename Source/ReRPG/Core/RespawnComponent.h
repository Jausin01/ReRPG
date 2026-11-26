#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RespawnComponent.generated.h"

class APlayerController;
class ARestPoint;
class UHealthComponent;
class UManaComponent;

// The player just died. Hook for the death animation.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathSequenceStarted);

// The player was brought back at the rest point (the screen is still black). Hook for the wake up animation.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRespawned);

// Handles the death of the player: locks the controls, fades the screen out, brings the player
// back at the last rest point with full health and mana, fades in and gives the control back.
// The sequence runs on timers and uses the events of other components: no tick.
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class RERPG_API URespawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URespawnComponent();

	// Where the player wakes up from now on. Called by the rest point.
	void SetRestPoint(const ARestPoint* NewPoint);

	// Where the player wakes up: the last rest point, or where the level started if none was used.
	FTransform GetRespawnTransform() const;

	// True from the death until the control is back.
	UFUNCTION(BlueprintPure, Category = "Respawn")
	bool IsRespawning() const { return bRespawning; }

	UPROPERTY(BlueprintAssignable, Category = "Respawn")
	FOnDeathSequenceStarted OnDeathSequenceStarted;

	UPROPERTY(BlueprintAssignable, Category = "Respawn")
	FOnRespawned OnRespawned;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleDeath(AActor* Killer);

	// Starts the fade to black after the player had time to see the death.
	void BeginFadeOut();

	// The screen is black: moves the player and restores health and mana, then fades in.
	void Respawn();

	// The screen is clear again: gives the control back.
	void FinishRespawn();

	APlayerController* GetPlayerController() const;

	// Time between the death and the start of the fade (the death is visible).
	UPROPERTY(EditDefaultsOnly, Category = "Respawn", meta = (ClampMin = "0.0"))
	float DeathLingerTime = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Respawn", meta = (ClampMin = "0.0"))
	float FadeOutTime = 1.f;

	// Time the screen stays black before the player wakes up.
	UPROPERTY(EditDefaultsOnly, Category = "Respawn", meta = (ClampMin = "0.0"))
	float BlackHoldTime = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Respawn", meta = (ClampMin = "0.0"))
	float FadeInTime = 1.5f;

	FTimerHandle SequenceTimer;
	TWeakObjectPtr<const ARestPoint> RestPoint;
	TWeakObjectPtr<UHealthComponent> OwnerHealth;
	TWeakObjectPtr<UManaComponent> OwnerMana;
	FTransform DefaultTransform;
	bool bRespawning = false;
};