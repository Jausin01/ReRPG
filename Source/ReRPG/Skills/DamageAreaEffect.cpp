#include "Skills/DamageAreaEffect.h"

#include "Combat/HealthComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
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

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(DamageAreaEffect), false, Caster);
	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(
		Overlaps,
		Center,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(Radius),
		QueryParams);

	// A character has several collision pieces, so make sure each target is hit once.
	TSet<AActor*> HitActors;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (HitActor == nullptr || HitActors.Contains(HitActor))
		{
			continue;
		}
		HitActors.Add(HitActor);

		UHealthComponent* Health = HitActor->FindComponentByClass<UHealthComponent>();
		if (Health != nullptr)
		{
			Health->ApplyDamage(Damage, Caster);
		}
	}
}