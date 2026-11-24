#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DodgeComponent.generated.h"

class ACharacter;
class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDodgeStarted, FVector, Direction);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class RERPG_API UDodgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDodgeComponent();

	// Starts a dodge if allowed. Returns true if the dodge started.
	UFUNCTION(BlueprintCallable, Category = "Dodge")
	bool RequestDodge();

	UFUNCTION(BlueprintPure, Category = "Dodge")
	bool IsDodging() const { return bIsDodging; }

	// Reserved for animation and VFX. Direction is the horizontal dodge direction.
	UPROPERTY(BlueprintAssignable, Category = "Dodge")
	FOnDodgeStarted OnDodgeStarted;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool CanDodge() const;
	void FinishDodge();

	// Initial horizontal speed of the dodge (ground friction slows it down afterwards).
	UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (ClampMin = "0.0"))
	float DodgeSpeed = 1000.f;

	// How long the dodge lasts. The owner is invulnerable for this whole time.
	UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (ClampMin = "0.05"))
	float DodgeDuration = 0.3f;

	// Minimum time between two dodge starts (counted from the start of the dodge).
	UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (ClampMin = "0.0"))
	float Cooldown = 0.8f;

	FTimerHandle DodgeEndTimer;
	TWeakObjectPtr<ACharacter> OwnerCharacter;
	TWeakObjectPtr<UHealthComponent> OwnerHealth;
	float NextDodgeTime = 0.f;
	bool bIsDodging = false;
};