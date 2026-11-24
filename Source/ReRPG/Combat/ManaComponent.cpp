#include "Combat/ManaComponent.h"

#include "Engine/World.h"
#include "TimerManager.h"

UManaComponent::UManaComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UManaComponent::BeginPlay()
{
	Super::BeginPlay();

	Mana = MaxMana;

	// Announce the starting value so listeners bound before BeginPlay (like the HUD) show the right amount.
	OnManaChanged.Broadcast(Mana, MaxMana, 0.f);
}

void UManaComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RegenTimer);
	}

	Super::EndPlay(EndPlayReason);
}

bool UManaComponent::TrySpend(float Amount)
{
	if (Amount <= 0.f || Mana < Amount)
	{
		return false;
	}

	SetMana(Mana - Amount);
	StartRegen(RegenDelay);
	return true;
}

float UManaComponent::Restore(float Amount)
{
	if (Amount <= 0.f)
	{
		return 0.f;
	}

	const float Before = Mana;
	SetMana(Mana + Amount);
	return Mana - Before;
}

void UManaComponent::SetRegenEnabled(bool bEnabled)
{
	if (bRegenEnabled == bEnabled)
	{
		return;
	}

	bRegenEnabled = bEnabled;

	if (bRegenEnabled)
	{
		StartRegen(RegenDelay);
	}
	else if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RegenTimer);
	}
}

void UManaComponent::SetMana(float NewMana)
{
	const float Clamped = FMath::Clamp(NewMana, 0.f, MaxMana);
	const float Delta = Clamped - Mana;
	if (FMath::IsNearlyZero(Delta))
	{
		return;
	}

	Mana = Clamped;
	OnManaChanged.Broadcast(Mana, MaxMana, Delta);
}

void UManaComponent::StartRegen(float InitialDelay)
{
	const UWorld* World = GetWorld();
	if (World == nullptr || !bRegenEnabled || RegenPerSecond <= 0.f || Mana >= MaxMana)
	{
		return;
	}

	// Restarts the timer: spending again during regen waits for the delay once more.
	World->GetTimerManager().SetTimer(RegenTimer, this, &UManaComponent::RegenStep, RegenInterval, true, InitialDelay);
}

void UManaComponent::RegenStep()
{
	SetMana(Mana + RegenPerSecond * RegenInterval);

	// Stop the timer once full so the component costs nothing while idle.
	if (Mana >= MaxMana)
	{
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(RegenTimer);
		}
	}
}