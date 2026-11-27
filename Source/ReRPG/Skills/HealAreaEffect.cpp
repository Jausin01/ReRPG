#include "Skills/HealAreaEffect.h"

#include "Combat/CombatTargeting.h"
#include "Combat/HealthComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

void UHealAreaEffect::Execute(AActor* Caster) const
{
	if (Caster == nullptr)
	{
		return;
	}

	// An effect is a plain UObject without a world, so the world comes from the caster.
	UWorld* World = Caster->GetWorld();
	if (World == nullptr)
	{
		return;
	}

	const FVector Center = Caster->GetActorLocation();

#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		DrawDebugSphere(World, Center, Radius, 16, FColor::Green, false, 0.5f);
	}
#endif

	// Only pawns of the caster's team are healed: enemies and neutral pawns are skipped.
	TArray<TWeakObjectPtr<UHealthComponent>> Targets;
	CombatTargeting::FindAllyTargets(Caster, Center, Radius, bIncludeCaster, Targets);

	for (const TWeakObjectPtr<UHealthComponent>& Target : Targets)
	{
		if (UHealthComponent* Health = Target.Get())
		{
			Health->Heal(Amount);
		}
	}
}