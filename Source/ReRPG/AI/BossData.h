#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BossData.generated.h"

class UBehaviorTree;
class USkillData;

// How a boss chooses who to fight in a phase. Used by the boss AI (it comes with the AI scripts).
UENUM(BlueprintType)
enum class EBossFocus : uint8
{
	// Keeps the current target.
	Default,

	Nearest,

	LowestHealth
};

// How a boss chooses its next skill in a phase.
UENUM(BlueprintType)
enum class EBossSkillSelection : uint8
{
	// Any skill that is ready, picked at random (unpredictable boss).
	Random,

	// The skills in the order of the list (a boss the player can learn).
	// Waits if the next skill is still on cooldown.
	Sequential
};

// One phase of a boss fight. Everything that changes between phases is data here.
USTRUCT(BlueprintType)
struct FBossPhase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Phase")
	FText PhaseName;

	// The phase starts when the health drops to this percentage (0 to 100).
	// Keep the list ordered from the highest to the lowest threshold.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Phase", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float HealthThreshold = 100.f;

	// Health recovered when the phase starts, as a percentage of the maximum (100 = full heal).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Phase", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float HealOnEnter = 0.f;

	// Time the boss is invulnerable while the phase starts (roar, animation). 0 = no pause.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Phase", meta = (ClampMin = "0.0"))
	float TransitionTime = 1.5f;

	// Scales the walk speed the boss has when the fight starts.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Phase", meta = (ClampMin = "0.1"))
	float SpeedMultiplier = 1.f;

	// Skills the boss can use in this phase (used by the Use Boss Skill task).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Phase")
	TArray<TObjectPtr<USkillData>> Skills;

	// How the boss picks the next skill of this phase.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Phase")
	EBossSkillSelection SkillSelection = EBossSkillSelection::Random;

	// Behavior Tree with the attack pattern of this phase. Used by the boss AI.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Phase")
	TObjectPtr<UBehaviorTree> AttackPattern;

	// How the boss chooses its target in this phase. Used by the boss AI.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Phase")
	EBossFocus Focus = EBossFocus::Default;
};

// Pure data describing the phases of one boss. Create one asset per boss in the editor.
UCLASS(BlueprintType)
class RERPG_API UBossData : public UDataAsset
{
	GENERATED_BODY()

public:
	const TArray<FBossPhase>& GetPhases() const { return Phases; }

private:
	// Ordered from the first phase (highest threshold) to the last.
	UPROPERTY(EditAnywhere, Category = "Boss", meta = (TitleProperty = "PhaseName", AllowPrivateAccess = "true"))
	TArray<FBossPhase> Phases;
};