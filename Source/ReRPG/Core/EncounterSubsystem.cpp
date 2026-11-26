#include "Core/EncounterSubsystem.h"

#include "AI/EnemyInfoComponent.h"

void UEncounterSubsystem::SetBossInCombat(UEnemyInfoComponent* Boss, bool bInCombat)
{
	if (Boss == nullptr)
	{
		return;
	}

	UEnemyInfoComponent* PreviousActive = GetActiveBoss();

	// Drop stale entries and any previous entry of this boss, then add it again if it is fighting.
	EngagedBosses.RemoveAll([Boss](const TWeakObjectPtr<UEnemyInfoComponent>& Entry)
		{
			return !Entry.IsValid() || Entry.Get() == Boss;
		});

	if (bInCombat)
	{
		EngagedBosses.Add(Boss);
	}

	// Only tell the HUD when what it shows actually changes.
	UEnemyInfoComponent* NewActive = GetActiveBoss();
	if (NewActive != PreviousActive)
	{
		OnActiveBossChanged.Broadcast(NewActive);
	}
}

UEnemyInfoComponent* UEncounterSubsystem::GetActiveBoss() const
{
	for (int32 i = EngagedBosses.Num() - 1; i >= 0; --i)
	{
		if (UEnemyInfoComponent* Boss = EngagedBosses[i].Get())
		{
			return Boss;
		}
	}

	return nullptr;
}

void UEncounterSubsystem::ResetEncounters()
{
	OnEncountersReset.Broadcast();
}