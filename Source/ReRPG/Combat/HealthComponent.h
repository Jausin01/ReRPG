#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChanged, float, NewHealth, float, MaxHealth, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, Killer);

// Fired when damage is actually applied (not when it is blocked). Amount is positive.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageTaken, float, Amount, AActor*, DamageCauser);

// Lets another component consume incoming damage before it is applied (parry, shield...).
// Return true to block the damage completely.
DECLARE_DELEGATE_RetVal_TwoParams(bool, FDamageInterceptor, float /*Amount*/, AActor* /*DamageCauser*/);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class RERPG_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	UFUNCTION(BlueprintCallable, Category = "Health")
	float ApplyDamage(float Amount, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Health")
	float Heal(float Amount);

	// Brings the owner back with the given percent of the maximum health, even from death.
	// It never lowers the current health. Also clears invulnerability and the damage multiplier.
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Revive(float HealthPercent = 100.f);

	// While invulnerable, ApplyDamage ignores all damage (used by dodge, cutscenes, etc.).
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetInvulnerable(bool bNewInvulnerable) { bInvulnerable = bNewInvulnerable; }

	// Scales all incoming damage (1 = normal, above 1 = vulnerable). Used by stun.
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetDamageMultiplier(float NewMultiplier) { DamageMultiplier = FMath::Max(0.f, NewMultiplier); }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsInvulnerable() const { return bInvulnerable; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetDamageMultiplier() const { return DamageMultiplier; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return Health <= 0.f; }

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

	// Broadcast before OnDeath, so on a killing blow Health is already 0 when listeners run.
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDamageTaken OnDamageTaken;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeath OnDeath;

	// Single-cast, C++ only. Bound by the component that owns the blocking logic.
	FDamageInterceptor DamageInterceptor;

protected:
	virtual void BeginPlay() override;

private:
	void SetHealth(float NewHealth, AActor* DamageCauser);

	UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	float Health = 0.f;
	float DamageMultiplier = 1.f;
	bool bInvulnerable = false;
};