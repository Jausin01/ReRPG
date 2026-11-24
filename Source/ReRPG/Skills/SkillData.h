#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SkillData.generated.h"

class AActor;
class UTexture2D;
class USkillEffect;

// Pure data describing one skill. Create one asset per skill in the editor.
UCLASS(BlueprintType)
class RERPG_API USkillData : public UDataAsset
{
	GENERATED_BODY()

public:
	// Runs every effect of this skill for the given caster.
	void ExecuteEffects(AActor* Caster) const;

	const FText& GetDisplayName() const { return DisplayName; }
	float GetManaCost() const { return ManaCost; }
	float GetCooldown() const { return Cooldown; }

	// Soft reference: the icon is only loaded when the UI actually needs it.
	const TSoftObjectPtr<UTexture2D>& GetIcon() const { return Icon; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill", meta = (AllowPrivateAccess = "true"))
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float ManaCost = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float Cooldown = 5.f;

	// Effects created inline in the asset, executed in order.
	UPROPERTY(EditAnywhere, Instanced, Category = "Skill", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<USkillEffect>> Effects;
};