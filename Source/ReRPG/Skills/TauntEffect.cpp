#include "Skills/TauntEffect.h"

#include "Combat/CombatTargeting.h"
#include "Combat/HealthComponent.h"
#include "Combat/ThreatComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

void UTauntEffect::Execute(AActor* Caster) const
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
		DrawDebugSphere(World, Center, Radius, 16, FColor::Yellow, false, 0.5f);
	}
#endif

	TArray<TWeakObjectPtr<UHealthComponent>> Targets;
	CombatTargeting::FindHostileTargets(Caster, Center, Radius, Targets);

	for (const TWeakObjectPtr<UHealthComponent>& Target : Targets)
	{
		const UHealthComponent* Health = Target.Get();
		const AActor* Enemy = Health != nullptr ? Health->GetOwner() : nullptr;
		if (Enemy == nullptr)
		{
			continue;
		}

		// Enemies without a threat component do not use threat, so there is nothing to pull.
		if (UThreatComponent* Threat = Enemy->FindComponentByClass<UThreatComponent>())
		{
			Threat->Taunt(Caster, Duration);
		}
	}
}