#include "Combat/MeleeAttackComponent.h"

#include "Combat/HealthComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

UMeleeAttackComponent::UMeleeAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	AttackSteps.SetNum(3);
	AttackSteps[0].Damage = 10.f;
	AttackSteps[1].Damage = 12.f;
	AttackSteps[2].Damage = 20.f;
}

void UMeleeAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AttackEndTimer);
		World->GetTimerManager().ClearTimer(ComboResetTimer);
	}

	Super::EndPlay(EndPlayReason);
}

void UMeleeAttackComponent::RequestAttack()
{
	UWorld* World = GetWorld();
	if (bIsAttacking || World == nullptr || !AttackSteps.IsValidIndex(ComboIndex))
	{
		return;
	}

	const FMeleeAttackStep& Step = AttackSteps[ComboIndex];
	bIsAttacking = true;

	FTimerManager& TimerManager = World->GetTimerManager();
	TimerManager.ClearTimer(ComboResetTimer);
	TimerManager.SetTimer(AttackEndTimer, this, &UMeleeAttackComponent::FinishAttack, Step.Duration, false);

	PerformHit(Step);
	OnAttackStarted.Broadcast(ComboIndex);
}

void UMeleeAttackComponent::FinishAttack()
{
	bIsAttacking = false;

	UWorld* World = GetWorld();
	if (World == nullptr || !AttackSteps.IsValidIndex(ComboIndex))
	{
		ResetCombo();
		return;
	}

	const float ComboWindow = AttackSteps[ComboIndex].ComboWindow;
	ComboIndex = (ComboIndex + 1) % AttackSteps.Num();

	World->GetTimerManager().SetTimer(ComboResetTimer, this, &UMeleeAttackComponent::ResetCombo, ComboWindow, false);
}

void UMeleeAttackComponent::ResetCombo()
{
	ComboIndex = 0;
}

void UMeleeAttackComponent::PerformHit(const FMeleeAttackStep& Step) const
{
	const AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (Owner == nullptr || World == nullptr)
	{
		return;
	}

	const FVector Center = Owner->GetActorLocation() + Owner->GetActorForwardVector() * Step.Range;

#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		DrawDebugSphere(World, Center, Step.Radius, 16, FColor::Red, false, 0.3f);
	}
#endif

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MeleeAttack), false, Owner);
	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(
		Overlaps,
		Center,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(Step.Radius),
		QueryParams);

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
			Health->ApplyDamage(Step.Damage, GetOwner());
		}
	}
}