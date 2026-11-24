#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MeleeAttackComponent.generated.h"

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.05"))
	float Duration = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.05"))
	float ComboWindow = 0.5f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackStarted, int32, ComboIndex);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class RERPG_API UMeleeAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMeleeAttackComponent();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void RequestAttack();

	UFUNCTION(BlueprintPure, Category = "Attack")
	bool IsAttacking() const { return bIsAttacking; }

	UPROPERTY(BlueprintAssignable, Category = "Attack")
	FOnAttackStarted OnAttackStarted;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void PerformHit(const FMeleeAttackStep& Step) const;
	void FinishAttack();
	void ResetCombo();

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	TArray<FMeleeAttackStep> AttackSteps;

	UPROPERTY(EditDefaultsOnly, Category = "Attack|Debug")
	bool bDrawDebug = true;

	FTimerHandle AttackEndTimer;
	FTimerHandle ComboResetTimer;
	int32 ComboIndex = 0;
	bool bIsAttacking = false;
};