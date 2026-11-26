#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EncounterSubsystem.generated.h"

class UEnemyInfoComponent;

// Boss currently shown on the HUD, or null when no boss is in combat.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveBossChanged, UEnemyInfoComponent*, Boss);

// Tracks which bosses are fighting the player so the HUD can show the big boss bar without
// searching for enemies. Bosses report their combat state here; the HUD only listens.
UCLASS()
class RERPG_API UEncounterSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Called by the boss's EnemyInfoComponent when its combat starts or ends.
	void SetBossInCombat(UEnemyInfoComponent* Boss, bool bInCombat);

	// The most recent boss still in combat, or null.
	UEnemyInfoComponent* GetActiveBoss() const;

	UPROPERTY(BlueprintAssignable, Category = "Encounter")
	FOnActiveBossChanged OnActiveBossChanged;

private:
	// In engagement order: the last valid entry is the active boss.
	TArray<TWeakObjectPtr<UEnemyInfoComponent>> EngagedBosses;
};