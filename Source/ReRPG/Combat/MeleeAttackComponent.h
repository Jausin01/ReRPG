#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MeleeAttackComponent.generated.h"

class UHealthComponent;
class UStunComponent;

USTRUCT(BlueprintType)
struct FMeleeAttackStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float Damage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float Range = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "1.0"))
	float Radius = 80.f;

	// Windup: time between the attack starting and the damage being applied.
	// This is the window the target has to react (dodge or parry). Clamped to Duration.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float HitDelay = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.05"))
	float Duration = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.05"))
	float ComboWindow = 0.5f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackStarted, int32, ComboIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackCanceled);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class RERPG_API UMeleeAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMeleeAttackComponent();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void RequestAttack();

	// Interrupts the current attack before it hits and resets the combo.
	UFUNCTION(BlueprintCallable, Category = "Attack")
	void CancelAttack();

	UFUNCTION(BlueprintPure, Category = "Attack")
	bool IsAttacking() const { return bIsAttacking; }

	UPROPERTY(BlueprintAssignable, Category = "Attack")
	FOnAttackStarted OnAttackStarted;

	// Reserved for animation: stop the attack animation.
	UPROPERTY(BlueprintAssignable, Category = "Attack")
	FOnAttackCanceled OnAttackCanceled;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool CanAttack() const;
	void PerformHit(const FMeleeAttackStep& Step) const;
	void ApplyPendingHit();
	void FinishAttack();
	void ResetCombo();

	UFUNCTION()
	void HandleStunStarted(float Duration);

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	TArray<FMeleeAttackStep> AttackSteps;

	UPROPERTY(EditDefaultsOnly, Category = "Attack|Debug")
	bool bDrawDebug = true;

	FTimerHandle HitTimer;
	FTimerHandle AttackEndTimer;
	FTimerHandle ComboResetTimer;
	TWeakObjectPtr<UHealthComponent> OwnerHealth;
	TWeakObjectPtr<UStunComponent> OwnerStun;
	int32 ComboIndex = 0;
	bool bIsAttacking = false;
};