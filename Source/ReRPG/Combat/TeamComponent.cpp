#include "Combat/TeamComponent.h"

#include "GameFramework/Actor.h"

UTeamComponent::UTeamComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

ECombatTeam UTeamComponent::GetTeamOf(const AActor* Actor)
{
	if (Actor == nullptr)
	{
		return ECombatTeam::Neutral;
	}

	const UTeamComponent* TeamComp = Actor->FindComponentByClass<UTeamComponent>();
	return TeamComp != nullptr ? TeamComp->GetTeam() : ECombatTeam::Neutral;
}

bool UTeamComponent::AreHostile(const AActor* A, const AActor* B)
{
	const ECombatTeam TeamA = GetTeamOf(A);
	const ECombatTeam TeamB = GetTeamOf(B);

	// Neutral never takes part in a fight, in either direction.
	return TeamA != ECombatTeam::Neutral && TeamB != ECombatTeam::Neutral && TeamA != TeamB;
}