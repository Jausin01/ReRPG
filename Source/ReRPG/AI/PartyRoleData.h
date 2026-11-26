#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PartyRoleData.generated.h"

// How a party member chooses who to attack. Used by the ally AI.
UENUM(BlueprintType)
enum class EPartyTargetRule : uint8
{
	// The enemy closest to the party member.
	Nearest,

	// The enemy with the least health (finishes the weak ones first).
	LowestHealth
};

// Pure data describing one party role (tank, mage, striker...). Create one asset per role in the
// editor and assign it to the Party Member component of the character. The same code runs every
// role; only these values change.
UCLASS(BlueprintType)
class RERPG_API UPartyRoleData : public UDataAsset
{
	GENERATED_BODY()

public:
	float GetThreatMultiplier() const { return ThreatMultiplier; }
	EPartyTargetRule GetTargetRule() const { return TargetRule; }
	bool CanMelee() const { return bCanMelee; }
	float GetMinRange() const { return FMath::Min(MinRange, MaxRange); }
	float GetMaxRange() const { return FMath::Max(MinRange, MaxRange); }
	float GetFollowDistance() const { return FollowDistance; }

private:
	// How much threat the damage of this member creates on enemies (a tank is high, a mage is low).
	UPROPERTY(EditAnywhere, Category = "Role", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float ThreatMultiplier = 1.f;

	// How this member picks its target.
	UPROPERTY(EditAnywhere, Category = "Role", meta = (AllowPrivateAccess = "true"))
	EPartyTargetRule TargetRule = EPartyTargetRule::Nearest;

	// True when the member fights with its melee attack. A mage turns this off and keeps its distance.
	UPROPERTY(EditAnywhere, Category = "Role", meta = (AllowPrivateAccess = "true"))
	bool bCanMelee = true;

	// Distance the member tries to keep from its target: it backs away when the target is closer
	// than MinRange and approaches when it is farther than MaxRange. A melee member uses 0 and a
	// value close to its attack range.
	UPROPERTY(EditAnywhere, Category = "Role|Position", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float MinRange = 0.f;

	UPROPERTY(EditAnywhere, Category = "Role|Position", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float MaxRange = 130.f;

	// When there is no fight, the member stays within this distance of the player.
	UPROPERTY(EditAnywhere, Category = "Role|Position", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float FollowDistance = 350.f;
};