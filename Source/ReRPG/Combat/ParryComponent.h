#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ParryComponent.generated.h"

class AActor;
class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnParryStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnParrySuccess, AActor*, Attacker);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class RERPG_API UParryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UParryComponent();

	// Opens the parry window if allowed. Returns true if the window opened.
	UFUNCTION(BlueprintCallable, Category = "Parry")
	bool RequestParry();

	UFUNCTION(BlueprintPure, Category = "Parry")
	bool IsParrying() const { return bIsParrying; }

	// Reserved for animation and VFX (the parry stance).
	UPROPERTY(BlueprintAssignable, Category = "Parry")
	FOnParryStarted OnParryStarted;

	// Fired when a hit lands inside the window. Attacker may be null.
	UPROPERTY(BlueprintAssignable, Category = "Parry")
	FOnParrySuccess OnParrySuccess;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Bound to UHealthComponent::DamageInterceptor. Returns true if the damage was consumed.
	bool InterceptDamage(float Amount, AActor* DamageCauser);
	void EndParry();

	// How long incoming hits count as a parry.
	UPROPERTY(EditDefaultsOnly, Category = "Parry", meta = (ClampMin = "0.05"))
	float ParryWindow = 0.2f;

	// Minimum time between two parry starts (counted from the start of the window).
	UPROPERTY(EditDefaultsOnly, Category = "Parry", meta = (ClampMin = "0.0"))
	float Cooldown = 0.6f;

	FTimerHandle ParryEndTimer;
	TWeakObjectPtr<UHealthComponent> OwnerHealth;
	float NextParryTime = 0.f;
	bool bIsParrying = false;
};