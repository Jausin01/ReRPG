#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StunComponent.generated.h"

class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStunStarted, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStunEnded);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class RERPG_API UStunComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStunComponent();

	// Stuns the owner and makes it take extra damage for the duration.
	// Calling it while already stunned refreshes the timer and the multiplier.
	UFUNCTION(BlueprintCallable, Category = "Stun")
	void ApplyStun(float Duration, float DamageMultiplier = 1.5f);

	UFUNCTION(BlueprintPure, Category = "Stun")
	bool IsStunned() const { return bIsStunned; }

	// Reserved for the special stun animation and VFX.
	UPROPERTY(BlueprintAssignable, Category = "Stun")
	FOnStunStarted OnStunStarted;

	UPROPERTY(BlueprintAssignable, Category = "Stun")
	FOnStunEnded OnStunEnded;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void EndStun();

	FTimerHandle StunEndTimer;
	TWeakObjectPtr<UHealthComponent> OwnerHealth;
	bool bIsStunned = false;
};