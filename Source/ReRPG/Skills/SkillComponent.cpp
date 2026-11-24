#include "Skills/SkillComponent.h"

#include "Combat/DodgeComponent.h"
#include "Combat/HealthComponent.h"
#include "Combat/ManaComponent.h"
#include "Combat/StunComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Skills/SkillData.h"

USkillComponent::USkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	Skills.SetNum(PageCount * SlotsPerPage);
}

void USkillComponent::BeginPlay()
{
	Super::BeginPlay();

	CooldownEndTimes.Init(0.f, Skills.Num());

	// Cache the sibling components once.
	const AActor* Owner = GetOwner();
	if (Owner != nullptr)
	{
		OwnerHealth = Owner->FindComponentByClass<UHealthComponent>();
		OwnerMana = Owner->FindComponentByClass<UManaComponent>();
		OwnerStun = Owner->FindComponentByClass<UStunComponent>();
		OwnerDodge = Owner->FindComponentByClass<UDodgeComponent>();
	}
}

bool USkillComponent::TryUseSkill(int32 PageSlot)
{
	const UWorld* World = GetWorld();
	if (World == nullptr || PageSlot < 0 || PageSlot >= SlotsPerPage)
	{
		return false;
	}

	const int32 SlotIndex = CurrentPage * SlotsPerPage + PageSlot;
	if (!Skills.IsValidIndex(SlotIndex) || !CooldownEndTimes.IsValidIndex(SlotIndex))
	{
		return false;
	}

	const USkillData* Skill = Skills[SlotIndex];
	if (Skill == nullptr || !CanCast())
	{
		return false;
	}

	// Check the cooldown before spending anything.
	const float Now = World->GetTimeSeconds();
	if (Now < CooldownEndTimes[SlotIndex])
	{
		return false;
	}

	// Free skills skip the mana check (TrySpend rejects non-positive amounts).
	const float ManaCost = Skill->GetManaCost();
	if (ManaCost > 0.f)
	{
		UManaComponent* Mana = OwnerMana.Get();
		if (Mana == nullptr || !Mana->TrySpend(ManaCost))
		{
			return false;
		}
	}

	CooldownEndTimes[SlotIndex] = Now + Skill->GetCooldown();

	Skill->ExecuteEffects(GetOwner());
	OnSkillUsed.Broadcast(SlotIndex);
	return true;
}

void USkillComponent::TogglePage()
{
	CurrentPage = (CurrentPage + 1) % PageCount;
	OnPageChanged.Broadcast(CurrentPage);
}

USkillData* USkillComponent::GetSkillAt(int32 SlotIndex) const
{
	return Skills.IsValidIndex(SlotIndex) ? Skills[SlotIndex].Get() : nullptr;
}

float USkillComponent::GetCooldownRemaining(int32 SlotIndex) const
{
	const UWorld* World = GetWorld();
	if (World == nullptr || !CooldownEndTimes.IsValidIndex(SlotIndex))
	{
		return 0.f;
	}

	return FMath::Max(0.f, CooldownEndTimes[SlotIndex] - World->GetTimeSeconds());
}

bool USkillComponent::CanCast() const
{
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

	const UDodgeComponent* Dodge = OwnerDodge.Get();
	if (Dodge != nullptr && Dodge->IsDodging())
	{
		return false;
	}

	return true;
}