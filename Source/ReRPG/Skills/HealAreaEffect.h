#pragma once

#include "CoreMinimal.h"
#include "Skills/SkillEffect.h"
#include "HealAreaEffect.generated.h"

// Heals every allied pawn with health inside a sphere around the caster.
UCLASS(meta = (DisplayName = "Heal Area"))
class RERPG_API UHealAreaEffect : public USkillEffect
{
	GENERATED_BODY()

public:
	virtual void Execute(AActor* Caster) const override;

private:
	// Health restored to each ally.
	UPROPERTY(EditAnywhere, Category = "Heal Area", meta = (ClampMin = "0.0"))
	float Amount = 30.f;

	// Radius of the sphere around the caster.
	UPROPERTY(EditAnywhere, Category = "Heal Area", meta = (ClampMin = "1.0"))
	float Radius = 400.f;

	// The caster heals itself too.
	UPROPERTY(EditAnywhere, Category = "Heal Area")
	bool bIncludeCaster = true;

	UPROPERTY(EditAnywhere, Category = "Heal Area|Debug")
	bool bDrawDebug = true;
};