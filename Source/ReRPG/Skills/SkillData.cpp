#include "Skills/SkillData.h"

#include "Skills/SkillEffect.h"

void USkillData::ExecuteEffects(AActor* Caster) const
{
	if (Caster == nullptr)
	{
		return;
	}

	for (const TObjectPtr<USkillEffect>& Effect : Effects)
	{
		if (Effect != nullptr)
		{
			Effect->Execute(Caster);
		}
	}
}