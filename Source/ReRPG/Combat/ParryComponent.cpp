#include "Combat/ParryComponent.h"

#include "Combat/HealthComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

UParryComponent::UParryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UParryComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache the health component once and hook into its damage pipeline.
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	OwnerHealth = Owner->FindComponentByClass<UHealthComponent>();
	if (UHealthComponent* Health = OwnerHealth.Get())
	{
		Health->DamageInterceptor.BindUObject(this, &UParryComponent::InterceptDamage);
	}
}

void UParryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ParryEndTimer);
	}

	// Only remove the hook if it is ours.
	if (UHealthComponent* Health = OwnerHealth.Get())
	{
		if (Health->DamageInterceptor.IsBoundToObject(this))
		{
			Health->DamageInterceptor.Unbind();
		}
	}

	Super::EndPlay(EndPlayReason);
}

bool UParryComponent::RequestParry()
{
	UWorld* World = GetWorld();
	if (World == nullptr || bIsParrying || World->GetTimeSeconds() < NextParryTime)
	{
		return false;
	}

	const UHealthComponent* Health = OwnerHealth.Get();
	if (Health != nullptr && Health->IsDead())
	{
		return false;
	}

	bIsParrying = true;
	NextParryTime = World->GetTimeSeconds() + Cooldown;

	World->GetTimerManager().SetTimer(ParryEndTimer, this, &UParryComponent::EndParry, ParryWindow, false);

	OnParryStarted.Broadcast();
	return true;
}

bool UParryComponent::InterceptDamage(float Amount, AActor* DamageCauser)
{
	if (!bIsParrying)
	{
		return false;
	}

	// The hit is fully blocked. The attacker reaction (stun) will listen to this event.
	OnParrySuccess.Broadcast(DamageCauser);
	return true;
}

void UParryComponent::EndParry()
{
	bIsParrying = false;
}