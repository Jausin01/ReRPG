#include "Combat/CombatTargeting.h"

#include "Combat/HealthComponent.h"
#include "Combat/TeamComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

void CombatTargeting::FindHostileTargets(const AActor* Instigator, const FVector& Center, float Radius, TArray<TWeakObjectPtr<UHealthComponent>>& OutTargets)
{
	OutTargets.Reset();

	if (Instigator == nullptr)
	{
		return;
	}

	UWorld* World = Instigator->GetWorld();
	if (World == nullptr)
	{
		return;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HostileTargets), false, Instigator);
	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(
		Overlaps,
		Center,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(Radius),
		QueryParams);

	// A character has several collision pieces, so each actor is handled once.
	TSet<const AActor*> HandledActors;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		const AActor* HitActor = Overlap.GetActor();
		if (HitActor == nullptr || HandledActors.Contains(HitActor))
		{
			continue;
		}
		HandledActors.Add(HitActor);

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