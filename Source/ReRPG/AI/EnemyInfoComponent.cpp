#include "AI/EnemyInfoComponent.h"

#include "Core/EncounterSubsystem.h"
#include "Engine/World.h"

UEnemyInfoComponent::UEnemyInfoComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyInfoComponent::SetInCombat(bool bNewInCombat)
{
	if (bInCombat == bNewInCombat)
	{
		return;
	}

	bInCombat = bNewInCombat;
	OnCombatChanged.Broadcast(bInCombat);

	// Bosses are also announced to the world, which drives the big bar on the HUD.
	if (BarStyle == EEnemyBarStyle::Boss)
	{
		if (UEncounterSubsystem* Encounters = UWorld::GetSubsystem<UEncounterSubsystem>(GetWorld()))
		{
			Encounters->SetBossInCombat(this, bInCombat);
		}
	}
}

void UEnemyInfoComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// An enemy leaving the world must not stay on screen.
	SetInCombat(false);

	Super::EndPlay(EndPlayReason);
}