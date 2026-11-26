#include "AI/BossSkillComponent.h"

#include "AI/BossData.h"
#include "AI/BossPhaseComponent.h"
#include "Combat/HealthComponent.h"
#include "Combat/StunComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Skills/SkillData.h"
#include "TimerManager.h"

UBossSkillComponent::UBossSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBossSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	// Cache the sibling components once and listen to what interrupts a cast.
	BossPhase = Owner->FindComponentByClass<UBossPhaseComponent>();
	OwnerHealth = Owner->FindComponentByClass<UHealthComponent>();
	OwnerStun = Owner->FindComponentByClass<UStunComponent>();

	if (UBossPhaseComponent* Phase = BossPhase.Get())
	{
		Phase->OnPhaseChanged.AddDynamic(this, &UBossSkillComponent::HandlePhaseChanged);
	}

	if (UHealthComponent* Health = OwnerHealth.Get())
	{
		Health->OnDeath.AddDynamic(this, &UBossSkillComponent::HandleDeath);
	}

	if (UStunComponent* Stun = OwnerStun.Get())
	{
		Stun->OnStunStarted.AddDynamic(this, &UBossSkillComponent::HandleStunStarted);
	}
}

void UBossSkillComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}

	if (UBossPhaseComponent* Phase = BossPhase.Get())
	{
		Phase->OnPhaseChanged.RemoveDynamic(this, &UBossSkillComponent::HandlePhaseChanged);
	}

	if (UHealthComponent* Health = OwnerHealth.Get())
	{
		Health->OnDeath.RemoveDynamic(this, &UBossSkillComponent::HandleDeath);
	}

	if (UStunComponent* Stun = OwnerStun.Get())
	{
		Stun->OnStunStarted.RemoveDynamic(this, &UBossSkillComponent::HandleStunStarted);
	}

	Super::EndPlay(EndPlayReason);
}

bool UBossSkillComponent::TryUseSkill()
{
	const UWorld* World = GetWorld();
	const UBossPhaseComponent* Phase = BossPhase.Get();
	const FBossPhase* PhaseData = Phase != nullptr ? Phase->GetCurrentPhase() : nullptr;
	if (World == nullptr || PhaseData == nullptr || PhaseData->Skills.IsEmpty() || !CanCast())
	{
		return false;
	}

	const float Now = World->GetTimeSeconds();
	USkillData* Skill = PickSkill(*PhaseData, Now);
	if (Skill == nullptr)
	{
		return false;
	}

	// The cooldown starts with the cast, like the skills of the player.
	CooldownEndTimes.Add(TWeakObjectPtr<const USkillData>(Skill), Now + Skill->GetCooldown());
	CastingSkill = Skill;

	const float WindUpTime = Skill->GetWindUpTime();
	OnWindUpStarted.Broadcast(Skill, WindUpTime);

	if (WindUpTime > 0.f)
	{
		World->GetTimerManager().SetTimer(WindUpTimer, this, &UBossSkillComponent::ReleaseSkill, WindUpTime, false);
	}
	else
	{
		ReleaseSkill();
	}

	return true;
}

void UBossSkillComponent::CancelCast()
{
	USkillData* Skill = CastingSkill.Get();
	if (Skill == nullptr)
	{
		return;
	}

	CastingSkill.Reset();

	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WindUpTimer);
	}

	OnSkillInterrupted.Broadcast(Skill);
}

bool UBossSkillComponent::CanCast() const
{
	if (IsCasting())
	{
		return false;
	}

	const UHealthComponent* Health = OwnerHealth.Get();
	if (Health != nullptr && Health->IsDead())
	{
		return false;
	}

	const UStunComponent* Stun = OwnerStun.Get();
	if (Stun != nullptr && Stun->IsStunned())
	{
		return false;
	}

	const UBossPhaseComponent* Phase = BossPhase.Get();
	if (Phase != nullptr && Phase->IsTransitioning())
	{
		return false;
	}

	return true;
}

bool UBossSkillComponent::IsReady(const USkillData* Skill, float Now) const
{
	const float* EndTime = CooldownEndTimes.Find(TWeakObjectPtr<const USkillData>(Skill));
	return EndTime == nullptr || Now >= *EndTime;
}

USkillData* UBossSkillComponent::PickSkill(const FBossPhase& Phase, float Now)
{
	switch (Phase.SkillSelection)
	{
	case EBossSkillSelection::Sequential:
		return PickSequential(Phase.Skills, Now);

	case EBossSkillSelection::Random:
	default:
		return PickRandom(Phase.Skills, Now);
	}
}

USkillData* UBossSkillComponent::PickRandom(const TArray<TObjectPtr<USkillData>>& Skills, float Now) const
{
	TArray<USkillData*, TInlineAllocator<8>> Ready;
	for (const TObjectPtr<USkillData>& Skill : Skills)
	{
		if (Skill != nullptr && IsReady(Skill, Now))
		{
			Ready.Add(Skill);
		}
	}

	return Ready.IsEmpty() ? nullptr : Ready[FMath::RandRange(0, Ready.Num() - 1)];
}

USkillData* UBossSkillComponent::PickSequential(const TArray<TObjectPtr<USkillData>>& Skills, float Now)
{
	// Skips empty entries, but waits (returns null) while the next real skill is on cooldown.
	for (int32 i = 0; i < Skills.Num(); ++i)
	{
		const int32 Index = (SequenceIndex + i) % Skills.Num();
		USkillData* Skill = Skills[Index];
		if (Skill == nullptr)
		{
			continue;
		}

		if (!IsReady(Skill, Now))
		{
			return nullptr;
		}

		SequenceIndex = (Index + 1) % Skills.Num();
		return Skill;
	}

	return nullptr;
}

void UBossSkillComponent::ReleaseSkill()
{
	USkillData* Skill = CastingSkill.Get();

	// Clear first, so listeners already see the boss as free to act.
	CastingSkill.Reset();
	if (Skill == nullptr)
	{
		return;
	}

	Skill->ExecuteEffects(GetOwner());
	OnSkillReleased.Broadcast(Skill);
}

void UBossSkillComponent::HandleStunStarted(float Duration)
{
	CancelCast();
}

void UBossSkillComponent::HandleDeath(AActor* Killer)
{
	CancelCast();
}

void UBossSkillComponent::HandlePhaseChanged(int32 PhaseIndex)
{
	CancelCast();
	SequenceIndex = 0;
}