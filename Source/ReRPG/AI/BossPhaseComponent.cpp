#include "AI/BossPhaseComponent.h"

#include "AI/BossData.h"
#include "Combat/HealthComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

UBossPhaseComponent::UBossPhaseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBossPhaseComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (Owner == nullptr || BossData == nullptr || BossData->GetPhases().IsEmpty())
	{
		return;
	}

	OwnerHealth = Owner->FindComponentByClass<UHealthComponent>();

	// The speed multiplier of each phase is applied over the speed the boss starts with.
	if (const ACharacter* Character = Cast<ACharacter>(Owner))
	{
		UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
		OwnerMovement = Movement;
		if (Movement != nullptr)
		{
			BaseWalkSpeed = Movement->MaxWalkSpeed;
		}
	}

	if (UHealthComponent* Health = OwnerHealth.Get())
	{
		Health->OnHealthChanged.AddDynamic(this, &UBossPhaseComponent::HandleHealthChanged);
	}

	// The first phase starts with the fight: no heal and no transition.
	ApplyPhase(0, false);
}

void UBossPhaseComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}

	if (UHealthComponent* Health = OwnerHealth.Get())
	{
		Health->OnHealthChanged.RemoveDynamic(this, &UBossPhaseComponent::HandleHealthChanged);
	}

	Super::EndPlay(EndPlayReason);
}

const FBossPhase* UBossPhaseComponent::GetCurrentPhase() const
{
	if (BossData == nullptr)
	{
		return nullptr;
	}

	const TArray<FBossPhase>& Phases = BossData->GetPhases();
	return Phases.IsValidIndex(PhaseIndex) ? &Phases[PhaseIndex] : nullptr;
}

void UBossPhaseComponent::HandleHealthChanged(float NewHealth, float MaxHealth, float Delta)
{
	// Only damage starts a phase. A dead boss does not change phase and a boss that is
	// already changing phase waits for the transition to end.
	if (Delta >= 0.f || NewHealth <= 0.f || MaxHealth <= 0.f || bTransitioning || BossData == nullptr)
	{
		return;
	}

	const float HealthPercent = NewHealth / MaxHealth * 100.f;
	const TArray<FBossPhase>& Phases = BossData->GetPhases();

	// Search from the last phase backwards: if one hit crossed several thresholds, the deepest wins.
	// The phase only moves forward, so healing above a threshold never goes back to an older phase.
	for (int32 i = Phases.Num() - 1; i > PhaseIndex; --i)
	{
		if (HealthPercent <= Phases[i].HealthThreshold)
		{
			StartTransition(i);
			return;
		}
	}
}

void UBossPhaseComponent::StartTransition(int32 NewIndex)
{
	PendingPhaseIndex = NewIndex;
	bTransitioning = true;

	// Blocks further damage right away. This runs inside the health event, so the phase itself
	// (which heals the boss and fires events) is applied on the next tick, after that event ended.
	if (UHealthComponent* Health = OwnerHealth.Get())
	{
		Health->SetInvulnerable(true);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(this, &UBossPhaseComponent::ApplyPendingPhase);
	}
}

void UBossPhaseComponent::ApplyPendingPhase()
{
	if (BossData == nullptr || !BossData->GetPhases().IsValidIndex(PendingPhaseIndex))
	{
		FinishTransition();
		return;
	}

	ApplyPhase(PendingPhaseIndex, true);

	const float TransitionTime = BossData->GetPhases()[PendingPhaseIndex].TransitionTime;
	UWorld* World = GetWorld();
	if (World != nullptr && TransitionTime > 0.f)
	{
		World->GetTimerManager().SetTimer(TransitionTimer, this, &UBossPhaseComponent::FinishTransition, TransitionTime, false);
	}
	else
	{
		FinishTransition();
	}
}

void UBossPhaseComponent::FinishTransition()
{
	bTransitioning = false;

	if (UHealthComponent* Health = OwnerHealth.Get())
	{
		Health->SetInvulnerable(false);
	}

	OnTransitionFinished.Broadcast(PhaseIndex);
}

void UBossPhaseComponent::ApplyPhase(int32 NewIndex, bool bHeal)
{
	if (BossData == nullptr || !BossData->GetPhases().IsValidIndex(NewIndex))
	{
		return;
	}

	PhaseIndex = NewIndex;
	const FBossPhase& Phase = BossData->GetPhases()[NewIndex];

	if (UCharacterMovementComponent* Movement = OwnerMovement.Get())
	{
		if (BaseWalkSpeed > 0.f)
		{
			Movement->MaxWalkSpeed = BaseWalkSpeed * Phase.SpeedMultiplier;
		}
	}

	if (bHeal && Phase.HealOnEnter > 0.f)
	{
		if (UHealthComponent* Health = OwnerHealth.Get())
		{
			Health->Heal(Health->GetMaxHealth() * Phase.HealOnEnter / 100.f);
		}
	}

	OnPhaseChanged.Broadcast(PhaseIndex);
}