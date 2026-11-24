#include "Combat/StunComponent.h"

#include "Combat/HealthComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

UStunComponent::UStunComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStunComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache the health component once.
	const AActor* Owner = GetOwner();
	if (Owner != nullptr)
	{
		OwnerHealth = Owner->FindComponentByClass<UHealthComponent>();
	}
}

void UStunComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StunEndTimer);
	}

	// Never leave the owner permanently vulnerable.
	if (bIsStunned)
	{
		EndStun();
	}

	Super::EndPlay(EndPlayReason);
}

void UStunComponent::ApplyStun(float Duration, float DamageMultiplier)
{
	UWorld* World = GetWorld();
	if (World == nullptr || Duration <= 0.f)
	{
		return;
	}

	UHealthComponent* Health = OwnerHealth.Get();
	if (Health != nullptr)
	{
		if (Health->IsDead())
		{
			return;
		}

		Health->SetDamageMultiplier(DamageMultiplier);
	}

	// Restarts the timer if already stunned.
	World->GetTimerManager().SetTimer(StunEndTimer, this, &UStunComponent::EndStun, Duration, false);

	// Only announce the transition, not every refresh.
	if (!bIsStunned)
	{
		bIsStunned = true;
		OnStunStarted.Broadcast(Duration);
	}
}

void UStunComponent::EndStun()
{
	bIsStunned = false;

	UHealthComponent* Health = OwnerHealth.Get();
	if (Health != nullptr)
	{
		Health->SetDamageMultiplier(1.f);
	}

	OnStunEnded.Broadcast();
}