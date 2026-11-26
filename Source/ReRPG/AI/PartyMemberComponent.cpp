#include "AI/PartyMemberComponent.h"

#include "AI/PartyRoleData.h"
#include "Combat/ThreatComponent.h"
#include "GameFramework/Actor.h"

UPartyMemberComponent::UPartyMemberComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPartyMemberComponent::BeginPlay()
{
	Super::BeginPlay();

	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	if (Role == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s is a party member without a role."), *Owner->GetName());
		return;
	}

	// The role decides how much threat this member creates on enemies.
	if (UThreatComponent* Threat = Owner->FindComponentByClass<UThreatComponent>())
	{
		Threat->SetThreatMultiplier(Role->GetThreatMultiplier());

		// A party member picks its target by its role, not by who hit it, so it keeps no threat table.
		Threat->SetTracksThreat(false);

		UE_LOG(LogTemp, Log, TEXT("%s: role %s applied (threat x%.1f)."), *Owner->GetName(), *Role->GetName(), Role->GetThreatMultiplier());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has a role but no Threat component, so the role cannot change its threat."), *Owner->GetName());
	}
}