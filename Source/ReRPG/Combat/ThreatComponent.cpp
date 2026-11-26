#include "Combat/ThreatComponent.h"

#include "Combat/HealthComponent.h"
#include "Combat/TeamComponent.h"
#include "GameFramework/Actor.h"

UThreatComponent::UThreatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UThreatComponent::BeginPlay()
{
	Super::BeginPlay();

	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	OwnerHealth = Owner->FindComponentByClass<UHealthComponent>();
	if (UHealthComponent* Health = OwnerHealth.Get())
	{
		Health->OnDamageTaken.AddDynamic(this, &UThreatComponent::HandleDamageTaken);
	}
}

void UThreatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UHealthComponent* Health = OwnerHealth.Get())
	{
		Health->OnDamageTaken.RemoveDynamic(this, &UThreatComponent::HandleDamageTaken);
	}

	Super::EndPlay(EndPlayReason);
}

void UThreatComponent::HandleDamageTaken(float Amount, AActor* DamageCauser)
{
	// Only hostile pawns create threat (friendly fire and neutral causers are ignored).
	if (DamageCauser == nullptr || !UTeamComponent::AreHostile(GetOwner(), DamageCauser))
	{
		return;
	}

	// The multiplier belongs to the one who deals the damage.
	float Multiplier = 1.f;
	if (const UThreatComponent* CauserThreat = DamageCauser->FindComponentByClass<UThreatComponent>())
	{
		Multiplier = CauserThreat->GetThreatMultiplier();
	}

	AddThreat(DamageCauser, Amount * Multiplier);
}

void UThreatComponent::AddThreat(AActor* Source, float Amount)
{
	if (Source == nullptr || Amount <= 0.f)
	{
		return;
	}

	for (FThreatEntry& Entry : Entries)
	{
		if (Entry.Source.Get() == Source)
		{
			Entry.Threat += Amount;
			return;
		}
	}

	FThreatEntry NewEntry;
	NewEntry.Source = Source;
	NewEntry.Threat = Amount;
	Entries.Add(NewEntry);
}

AActor* UThreatComponent::PickTarget(const FVector& Origin, float MaxDistance, const AActor* CurrentTarget)
{
	// Forget sources that no longer exist.
	Entries.RemoveAll([](const FThreatEntry& Entry)
		{
			return !Entry.Source.IsValid();
		});

	AActor* Best = nullptr;
	float BestThreat = 0.f;
	float CurrentThreat = 0.f;

	for (const FThreatEntry& Entry : Entries)
	{
		AActor* Source = Entry.Source.Get();
		if (Source == nullptr || FVector::Dist2D(Origin, Source->GetActorLocation()) > MaxDistance)
		{
			continue;
		}

		// A dead source is not a target. A health component that has not begun play still holds
		// 0 health, so IsDead() is only trusted once it started.
		const UHealthComponent* Health = Source->FindComponentByClass<UHealthComponent>();
		if (Health != nullptr && Health->HasBegunPlay() && Health->IsDead())
		{
			continue;
		}

		if (Source == CurrentTarget)
		{
			CurrentThreat = Entry.Threat;
		}

		if (Entry.Threat > BestThreat)
		{
			BestThreat = Entry.Threat;
			Best = Source;
		}
	}

	// The current target is kept unless someone has clearly more threat than it.
	if (Best != nullptr && Best != CurrentTarget && BestThreat <= CurrentThreat * SwitchThreshold)
	{
		return nullptr;
	}

	return Best;
}

void UThreatComponent::ClearThreat()
{
	Entries.Reset();
}