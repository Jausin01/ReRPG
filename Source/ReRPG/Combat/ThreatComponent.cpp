#include "Combat/ThreatComponent.h"

#include "Combat/HealthComponent.h"
#include "Combat/TeamComponent.h"
#include "Engine/World.h"
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
	if (!bTracksThreat || DamageCauser == nullptr || !UTeamComponent::AreHostile(GetOwner(), DamageCauser))
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

void UThreatComponent::Taunt(AActor* Source, float Duration)
{
	if (Source == nullptr)
	{
		return;
	}

	float TopThreat = 0.f;
	float SourceThreat = 0.f;
	for (const FThreatEntry& Entry : Entries)
	{
		if (!Entry.Source.IsValid())
		{
			continue;
		}

		TopThreat = FMath::Max(TopThreat, Entry.Threat);
		if (Entry.Source.Get() == Source)
		{
			SourceThreat = Entry.Threat;
		}
	}

	// Enough to pass the switch threshold over the top threat, with a small margin so a tie
	// of rounding never keeps the enemy on its old target. AddThreat ignores non-positive amounts.
	// This is what keeps the enemy on the source after the forced time ends.
	const float Needed = TopThreat * SwitchThreshold * 1.1f + 1.f;
	AddThreat(Source, Needed - SourceThreat);

	// The forced time makes the taunt work right away, whatever the other targeting rules say.
	const UWorld* World = GetWorld();
	if (World != nullptr && Duration > 0.f)
	{
		ForcedTarget = Source;
		ForcedUntil = World->GetTimeSeconds() + Duration;
	}

	UE_LOG(LogTemp, Log, TEXT("%s taunted by %s (forced for %.1fs)."), *GetNameSafe(GetOwner()), *GetNameSafe(Source), Duration);
}

bool UThreatComponent::HasForcedTarget() const
{
	const UWorld* World = GetWorld();
	return World != nullptr && ForcedTarget.IsValid() && World->GetTimeSeconds() < ForcedUntil;
}

AActor* UThreatComponent::PickTarget(const FVector& Origin, float MaxDistance, const AActor* CurrentTarget)
{
	// A taunt comes before everything else, as long as the taunter can still be fought.
	if (HasForcedTarget())
	{
		AActor* Forced = ForcedTarget.Get();
		const UHealthComponent* ForcedHealth = Forced->FindComponentByClass<UHealthComponent>();
		const bool bForcedDead = ForcedHealth != nullptr && ForcedHealth->HasBegunPlay() && ForcedHealth->IsDead();

		if (!bForcedDead && FVector::Dist2D(Origin, Forced->GetActorLocation()) <= MaxDistance)
		{
			return Forced;
		}
	}

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
	ForcedTarget.Reset();
	ForcedUntil = 0.f;
}