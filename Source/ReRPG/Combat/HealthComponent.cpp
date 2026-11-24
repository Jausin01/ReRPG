#include "Combat/HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	// Announce the starting value so listeners bound before BeginPlay (like the HUD) show the right amount.
	OnHealthChanged.Broadcast(Health, MaxHealth, 0.f);
}

float UHealthComponent::GetHealthPercent() const
{
	return MaxHealth > 0.f ? Health / MaxHealth : 0.f;
}

float UHealthComponent::ApplyDamage(float Amount, AActor* DamageCauser)
{
	if (IsDead() || bInvulnerable || Amount <= 0.f)
	{
		return 0.f;
	}

	// Give other systems (parry) a chance to block the hit entirely.
	if (DamageInterceptor.IsBound() && DamageInterceptor.Execute(Amount, DamageCauser))
	{
		return 0.f;
	}

	const float HealthBefore = Health;
	SetHealth(Health - Amount * DamageMultiplier, DamageCauser);
	return HealthBefore - Health;
}

float UHealthComponent::Heal(float Amount)
{
	if (IsDead() || Amount <= 0.f)
	{
		return 0.f;
	}

	const float HealthBefore = Health;
	SetHealth(Health + Amount, nullptr);
	return Health - HealthBefore;
}

void UHealthComponent::SetHealth(float NewHealth, AActor* DamageCauser)
{
	const float ClampedHealth = FMath::Clamp(NewHealth, 0.f, MaxHealth);
	const float Delta = ClampedHealth - Health;

	if (FMath::IsNearlyZero(Delta))
	{
		return;
	}

	Health = ClampedHealth;
	OnHealthChanged.Broadcast(Health, MaxHealth, Delta);

	if (Health <= 0.f)
	{
		OnDeath.Broadcast(DamageCauser);
	}
}