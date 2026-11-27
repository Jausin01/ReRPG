#include "Combat/CombatTargeting.h"

#include "Combat/HealthComponent.h"
#include "Combat/TeamComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

namespace
{
	// Every pawn inside the sphere, each one listed once (a character has several collision
	// pieces). The instigator is left out when bIgnoreInstigator is true.
	void QueryPawns(const AActor* Instigator, const FVector& Center, float Radius, bool bIgnoreInstigator, TArray<const AActor*>& OutActors)
	{
		OutActors.Reset();

		if (Instigator == nullptr)
		{
			return;
		}

		UWorld* World = Instigator->GetWorld();
		if (World == nullptr)
		{
			return;
		}

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CombatTargets), false, bIgnoreInstigator ? Instigator : nullptr);
		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByObjectType(
			Overlaps,
			Center,
			FQuat::Identity,
			FCollisionObjectQueryParams(ECC_Pawn),
			FCollisionShape::MakeSphere(Radius),
			QueryParams);

		TSet<const AActor*> HandledActors;
		for (const FOverlapResult& Overlap : Overlaps)
		{
			const AActor* HitActor = Overlap.GetActor();
			if (HitActor == nullptr || HandledActors.Contains(HitActor))
			{
				continue;
			}

			HandledActors.Add(HitActor);
			OutActors.Add(HitActor);
		}
	}
}

void CombatTargeting::FindHostileTargets(const AActor* Instigator, const FVector& Center, float Radius, TArray<TWeakObjectPtr<UHealthComponent>>& OutTargets)
{
	OutTargets.Reset();

	TArray<const AActor*> Actors;
	QueryPawns(Instigator, Center, Radius, true, Actors);

	for (const AActor* HitActor : Actors)
	{
		if (!UTeamComponent::AreHostile(Instigator, HitActor))
		{
			continue;
		}

		if (UHealthComponent* Health = HitActor->FindComponentByClass<UHealthComponent>())
		{
			OutTargets.Add(Health);
		}
	}
}

void CombatTargeting::FindAllyTargets(const AActor* Instigator, const FVector& Center, float Radius, bool bIncludeInstigator, TArray<TWeakObjectPtr<UHealthComponent>>& OutTargets)
{
	OutTargets.Reset();

	// A neutral instigator has no allies.
	const ECombatTeam Team = UTeamComponent::GetTeamOf(Instigator);
	if (Team == ECombatTeam::Neutral)
	{
		return;
	}

	TArray<const AActor*> Actors;
	QueryPawns(Instigator, Center, Radius, !bIncludeInstigator, Actors);

	for (const AActor* HitActor : Actors)
	{
		if (UTeamComponent::GetTeamOf(HitActor) != Team)
		{
			continue;
		}

		if (UHealthComponent* Health = HitActor->FindComponentByClass<UHealthComponent>())
		{
			OutTargets.Add(Health);
		}
	}
}