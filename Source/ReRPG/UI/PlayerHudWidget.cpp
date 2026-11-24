#include "UI/PlayerHUDWidget.h"

#include "Combat/HealthComponent.h"
#include "Combat/ManaComponent.h"
#include "GameFramework/Pawn.h"
#include "UI/ResourceBarWidget.h"

void UPlayerHUDWidget::BindToPawn(APawn* Pawn)
{
	// Rebinding (a new pawn after respawn) must not leave the old listeners behind.
	Unbind();

	if (Pawn == nullptr)
	{
		return;
	}

	BoundHealth = Pawn->FindComponentByClass<UHealthComponent>();
	BoundMana = Pawn->FindComponentByClass<UManaComponent>();

	// Show the current values right away. If a component has not begun play yet, it
	// broadcasts its starting value later and the bar is corrected by the event.
	if (UHealthComponent* Health = BoundHealth.Get())
	{
		Health->OnHealthChanged.AddDynamic(this, &UPlayerHUDWidget::HandleHealthChanged);
		HandleHealthChanged(Health->GetHealth(), Health->GetMaxHealth(), 0.f);
	}

	if (UManaComponent* Mana = BoundMana.Get())
	{
		Mana->OnManaChanged.AddDynamic(this, &UPlayerHUDWidget::HandleManaChanged);
		HandleManaChanged(Mana->GetMana(), Mana->GetMaxMana(), 0.f);
	}
}

void UPlayerHUDWidget::Unbind()
{
	if (UHealthComponent* Health = BoundHealth.Get())
	{
		Health->OnHealthChanged.RemoveDynamic(this, &UPlayerHUDWidget::HandleHealthChanged);
	}

	if (UManaComponent* Mana = BoundMana.Get())
	{
		Mana->OnManaChanged.RemoveDynamic(this, &UPlayerHUDWidget::HandleManaChanged);
	}

	BoundHealth = nullptr;
	BoundMana = nullptr;
}

void UPlayerHUDWidget::NativeDestruct()
{
	Unbind();

	Super::NativeDestruct();
}

void UPlayerHUDWidget::HandleHealthChanged(float NewHealth, float MaxHealth, float Delta)
{
	if (HealthBar != nullptr)
	{
		HealthBar->SetValues(NewHealth, MaxHealth);
	}
}

void UPlayerHUDWidget::HandleManaChanged(float NewMana, float MaxMana, float Delta)
{
	if (ManaBar != nullptr)
	{
		ManaBar->SetValues(NewMana, MaxMana);
	}
}