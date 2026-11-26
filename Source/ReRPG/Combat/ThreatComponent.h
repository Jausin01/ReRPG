#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ThreatComponent.generated.h"

class UHealthComponent;

// One source of threat in the table of an enemy.
struct FThreatEntry
{
	TWeakObjectPtr<AActor> Source;
	float Threat = 0.f;
};

// Threat of a pawn, in two roles:
// - On an enemy it keeps a table of how much threat each hostile pawn created (damage taken),
//   and tells the AI which one it should be fighting.
// - On any pawn it sets how much threat its own damage creates (a tank creates a lot, a mage little).
// It only listens to the health events: no tick.
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class RERPG_API UThreatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UThreatComponent();

	// Adds threat of a source to the table. Ignored for null sources and non-positive amounts.
	void AddThreat(AActor* Source, float Amount);

	// Asks which source the enemy should be fighting, looking only at living sources inside
	// MaxDistance (ground plane). Returns:
	// - the source with the most threat when it beats the current target by the switch threshold
	//   (or when the current target has no threat);
	// - null when the table has no opinion (no valid source) or the current target should be kept.
	AActor* PickTarget(const FVector& Origin, float MaxDistance, const AActor* CurrentTarget);

	// Forgets every source (the fight is over).
	void ClearThreat();

	UFUNCTION(BlueprintPure, Category = "Threat")
	float GetThreatMultiplier() const { return ThreatMultiplier; }

	// Roles change this at runtime (a tank sets a high value).
	UFUNCTION(BlueprintCallable, Category = "Threat")
	void SetThreatMultiplier(float NewMultiplier) { ThreatMultiplier = FMath::Max(NewMultiplier, 0.f); }

	// Turns the table off: the damage this pawn takes stops creating threat on its attackers.
	// Party members use it because their target comes from their role.
	UFUNCTION(BlueprintCallable, Category = "Threat")
	void SetTracksThreat(bool bNewTracksThreat) { bTracksThreat = bNewTracksThreat; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Every point of damage taken from a hostile pawn becomes threat for that pawn.
	UFUNCTION()
	void HandleDamageTaken(float Amount, AActor* DamageCauser);

	// How much threat the damage dealt by this pawn creates (1 = the same as the damage).
	UPROPERTY(EditAnywhere, Category = "Threat", meta = (ClampMin = "0.0"))
	float ThreatMultiplier = 1.f;

	// How much more threat than the current target another source needs before the enemy
	// changes its target (1.25 = 25% more). Keeps the enemy from switching all the time.
	UPROPERTY(EditAnywhere, Category = "Threat", meta = (ClampMin = "1.0"))
	float SwitchThreshold = 1.25f;

	// When false, damage taken by this pawn is not added to the table.
	UPROPERTY(EditAnywhere, Category = "Threat")
	bool bTracksThreat = true;

	TArray<FThreatEntry> Entries;
	TWeakObjectPtr<UHealthComponent> OwnerHealth;
};