#include "AI/EnemyInfoComponent.h"

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
}