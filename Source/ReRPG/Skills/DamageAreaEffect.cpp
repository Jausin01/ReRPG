#include "Skills/DamageAreaEffect.h"

#include "Combat/CombatTargeting.h"
#include "Combat/HealthComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

void UDamageAreaEffect::Execute(AActor* Caster) const
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

	const FVector Center = Caster->GetActorLocation() + Caster->GetActorForwardVector() * Range;

#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		DrawDebugSphere(World, Center, Radius, 16, FColor::Orange, false, 0.5f);
	}
#endif

	// Only hostile pawns are hit: allies, neutral pawns and the caster itself are skipped.
	TArray<TWeakObjectPtr<UHealthComponent>> Targets;
	CombatTargeting::FindHostileTargets(Caster, Center, Radius, Targets);

	for (const TWeakObjectPtr<UHealthComponent>& Target : Targets)
	{
		if (UHealthComponent* Health = Target.Get())
		{
			Health->ApplyDamage(Damage, Caster);
		}
	}
}