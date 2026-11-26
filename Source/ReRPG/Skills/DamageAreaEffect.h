#pragma once

#include "CoreMinimal.h"
#include "Skills/SkillEffect.h"
#include "DamageAreaEffect.generated.h"

// Deals damage to every hostile pawn with health inside a sphere in front of the caster.
UCLASS(meta = (DisplayName = "Damage Area"))
class RERPG_API UDamageAreaEffect : public USkillEffect
{
	GENERATED_BODY()

public:
	virtual void Execute(AActor* Caster) const override;

private:
	UPROPERTY(EditAnywhere, Category = "Damage Area", meta = (ClampMin = "0.0"))
	float Damage = 30.f;

	// Distance from the caster to the center of the sphere.
	UPROPERTY(EditAnywhere, Category = "Damage Area", meta = (ClampMin = "0.0"))
	float Range = 150.f;

	UPROPERTY(EditAnywhere, Category = "Damage Area", meta = (ClampMin = "1.0"))
	float Radius = 150.f;

	UPROPERTY(EditAnywhere, Category = "Damage Area|Debug")
	bool bDrawDebug = true;
};