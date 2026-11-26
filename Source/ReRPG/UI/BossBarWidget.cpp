#include "UI/BossBarWidget.h"

#include "AI/EnemyInfoComponent.h"
#include "Combat/HealthComponent.h"
#include "Components/TextBlock.h"
#include "Core/EncounterSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UI/ResourceBarWidget.h"

void UBossBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Hidden until a boss starts fighting.
	SetVisibility(ESlateVisibility::Collapsed);

	if (UEncounterSubsystem* Encounters = UWorld::GetSubsystem<UEncounterSubsystem>(GetWorld()))
	{
		Encounters->OnActiveBossChanged.AddDynamic(this, &UBossBarWidget::HandleActiveBossChanged);

		// A boss may already be fighting when the HUD is created.
		HandleActiveBossChanged(Encounters->GetActiveBoss());
	}
}

void UBossBarWidget::NativeDestruct()
{
	if (UEncounterSubsystem* Encounters = UWorld::GetSubsystem<UEncounterSubsystem>(GetWorld()))
	{
		Encounters->OnActiveBossChanged.RemoveDynamic(this, &UBossBarWidget::HandleActiveBossChanged);
	}

	UnbindBoss();

	Super::NativeDestruct();
}

void UBossBarWidget::UnbindBoss()
{
	if (UHealthComponent* Health = BoundHealth.Get())
	{
		Health->OnHealthChanged.RemoveDynamic(this, &UBossBarWidget::HandleHealthChanged);
	}

	BoundHealth = nullptr;
}

void UBossBarWidget::HandleActiveBossChanged(UEnemyInfoComponent* Boss)
{
	// Always release the previous boss first.
	UnbindBoss();

	if (Boss == nullptr)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (BossNameText != nullptr)
	{
		BossNameText->SetText(Boss->GetBossName());
	}

	if (UHealthComponent* Health = Boss->GetOwner()->FindComponentByClass<UHealthComponent>())
	{
		BoundHealth = Health;
		Health->OnHealthChanged.AddDynamic(this, &UBossBarWidget::HandleHealthChanged);
		HandleHealthChanged(Health->GetHealth(), Health->GetMaxHealth(), 0.f);
	}

	// The bar is only for looking at, it must not block clicks.
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UBossBarWidget::HandleHealthChanged(float NewHealth, float MaxHealth, float Delta)
{
	if (HealthBar != nullptr)
	{
		HealthBar->SetValues(NewHealth, MaxHealth);
	}
}