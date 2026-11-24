#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SkillEffect.generated.h"

class AActor;

// Base class for everything a skill can do (area damage, projectile, buff, heal...).
// Subclasses are created inline inside a USkillData asset, so each skill is just data.
UCLASS(Abstract, EditInlineNew, DefaultToInstanced)
class RERPG_API USkillEffect : public UObject
{
	GENERATED_BODY()

public:
	// Runs the effect on behalf of the caster. Subclasses override this.
	virtual void Execute(AActor* Caster) const PURE_VIRTUAL(USkillEffect::Execute, );
};