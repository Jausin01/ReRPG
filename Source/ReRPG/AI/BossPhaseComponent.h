#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BossPhaseComponent.generated.h"

class UBossData;
class UCharacterMovementComponent;
class UHealthComponent;
struct FBossPhase;

// The phase is already the new one when this is broadcast.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossPhaseChanged, int32, PhaseIndex);

// Broadcast when the invulnerable transition of a phase ends and the boss can fight again.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossTransitionFinished, int32, PhaseIndex);

// Moves a boss through the phases of its BossData as it loses health. It only listens to the
// health events: no tick. The AI and the animation listen to its events.
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class RERPG_API UBossPhaseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBossPhaseComponent();

	UFUNCTION(BlueprintPure, Category = "Boss")
	int32 GetPhaseIndex() const { return PhaseIndex; }

	// True while the boss is invulnerable and busy changing phase.
	UFUNCTION(BlueprintPure, Category = "Boss")
	bool IsTransitioning() const { return bTransitioning; }

	// Data of the current phase, or null if there is no data.
	const FBossPhase* GetCurrentPhase() const;

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnBossPhaseChanged OnPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnBossTransitionFinished OnTransitionFinished;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Sets the phase and applies its data. Heals only when bHeal is true.
	void ApplyPhase(int32 NewIndex, bool bHeal);

	// Blocks damage right away and applies the new phase on the next tick.
	void StartTransition(int32 NewIndex);
	void ApplyPendingPhase();
	void FinishTransition();

	UFUNCTION()
	void HandleHealthChanged(float NewHealth, float MaxHealth, float Delta);

	UPROPERTY(EditAnywhere, Category = "Boss")
	TObjectPtr<UBossData> BossData;

	FTimerHandle TransitionTimer;
	TWeakObjectPtr<UHealthComponent> OwnerHealth;
	TWeakObjectPtr<UCharacterMovementComponent> OwnerMovement;
	float BaseWalkSpeed = 0.f;
	int32 PhaseIndex = INDEX_NONE;
	int32 PendingPhaseIndex = INDEX_NONE;
	bool bTransitioning = false;
};