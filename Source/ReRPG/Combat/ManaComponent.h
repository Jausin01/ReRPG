#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ManaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnManaChanged, float, NewMana, float, MaxMana, float, Delta);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class RERPG_API UManaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UManaComponent();

	// Spends mana if enough is available. Returns false (spending nothing) otherwise.
	UFUNCTION(BlueprintCallable, Category = "Mana")
	bool TrySpend(float Amount);

	// Restores mana. Returns the amount actually restored.
	UFUNCTION(BlueprintCallable, Category = "Mana")
	float Restore(float Amount);

	// Toggles passive regeneration (for effects, states, etc.).
	UFUNCTION(BlueprintCallable, Category = "Mana")
	void SetRegenEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Mana")
	float GetMana() const { return Mana; }

	UFUNCTION(BlueprintPure, Category = "Mana")
	float GetMaxMana() const { return MaxMana; }

	UFUNCTION(BlueprintPure, Category = "Mana")
	float GetManaPercent() const { return MaxMana > 0.f ? Mana / MaxMana : 0.f; }

	UFUNCTION(BlueprintPure, Category = "Mana")
	bool IsRegenEnabled() const { return bRegenEnabled; }

	UPROPERTY(BlueprintAssignable, Category = "Mana")
	FOnManaChanged OnManaChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void SetMana(float NewMana);
	void StartRegen(float InitialDelay);
	void RegenStep();

	UPROPERTY(EditDefaultsOnly, Category = "Mana", meta = (ClampMin = "1.0"))
	float MaxMana = 100.f;

	// Mana recovered per second.
	UPROPERTY(EditDefaultsOnly, Category = "Mana|Regen", meta = (ClampMin = "0.0"))
	float RegenPerSecond = 5.f;

	// Time without regeneration after spending mana.
	UPROPERTY(EditDefaultsOnly, Category = "Mana|Regen", meta = (ClampMin = "0.0"))
	float RegenDelay = 1.f;

	// Time between regen steps (larger = fewer calls and fewer events).
	UPROPERTY(EditDefaultsOnly, Category = "Mana|Regen", meta = (ClampMin = "0.05"))
	float RegenInterval = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Mana|Regen")
	bool bRegenEnabled = true;

	FTimerHandle RegenTimer;
	float Mana = 0.f;
};