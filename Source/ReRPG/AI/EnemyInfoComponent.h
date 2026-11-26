#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyInfoComponent.generated.h"

// How the health of an enemy is shown to the player.
UENUM(BlueprintType)
enum class EEnemyBarStyle : uint8
{
	// Small bar above the head. Minor enemies have no name (they get a bestiary entry later).
	Minor,

	// Large bar at the bottom center of the screen, with the boss name.
	Boss
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyCombatChanged, bool, bInCombat);

// Presentation data of an enemy plus its combat state. The AI controller decides when the
// combat starts and ends; the UI only listens to OnCombatChanged.
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class RERPG_API UEnemyInfoComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyInfoComponent();

	// Called by the AI controller. Broadcasts only when the state actually changes.
	void SetInCombat(bool bNewInCombat);

	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool IsInCombat() const { return bInCombat; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	EEnemyBarStyle GetBarStyle() const { return BarStyle; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	FText GetBossName() const { return BossName; }

	UPROPERTY(BlueprintAssignable, Category = "Enemy")
	FOnEnemyCombatChanged OnCombatChanged;

private:
	UPROPERTY(EditAnywhere, Category = "Enemy")
	EEnemyBarStyle BarStyle = EEnemyBarStyle::Minor;

	// Only bosses have a name on screen.
	UPROPERTY(EditAnywhere, Category = "Enemy", meta = (EditCondition = "BarStyle == EEnemyBarStyle::Boss", EditConditionHides))
	FText BossName;

	bool bInCombat = false;
};