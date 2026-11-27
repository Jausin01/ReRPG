#pragma once

#include "CoreMinimal.h"
#include "Skills/SkillEffect.h"
#include "TauntEffect.generated.h"

// Makes every hostile pawn with a threat table inside a sphere around the caster turn on the
// caster: the caster gets enough threat to pull them away from their current target, and they
// are forced to fight the caster for a few seconds.
UCLASS(meta = (DisplayName = "Taunt"))
class RERPG_API UTauntEffect : public USkillEffect
{
	GENERATED_BODY()

public:
	virtual void Execute(AActor* Caster) const override;

private:
	// Radius of the sphere around the caster.
	UPROPERTY(EditAnywhere, Category = "Taunt", meta = (ClampMin = "1.0"))
	float Radius = 600.f;

	// Seconds the enemies are forced to fight the caster. After that, the threat keeps them on it.
	UPROPERTY(EditAnywhere, Category = "Taunt", meta = (ClampMin = "0.0"))
	float Duration = 3.f;

	UPROPERTY(EditAnywhere, Category = "Taunt|Debug")
	bool bDrawDebug = true;
};