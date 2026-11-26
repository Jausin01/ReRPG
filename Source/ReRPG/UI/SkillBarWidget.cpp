#include "UI/SkillBarWidget.h"

#include "Combat/ManaComponent.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"
#include "Skills/SkillComponent.h"
#include "Skills/SkillData.h"
#include "UI/SkillSlotWidget.h"

// The layout has one widget per slot of a page; keep them in sync with the component.
static_assert(USkillComponent::SlotsPerPage == 4, "The skill bar layout has exactly 4 slots.");

void USkillBarWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SkillSlots.Reset();
	SkillSlots.Add(SkillSlot0);
	SkillSlots.Add(SkillSlot1);
	SkillSlots.Add(SkillSlot2);
	SkillSlots.Add(SkillSlot3);

	// Slots map to the skill keys 1 to 4.
	for (int32 i = 0; i < SkillSlots.Num(); ++i)
	{
		SkillSlots[i]->SetKeyLabel(FText::AsNumber(i + 1));
	}
}

void USkillBarWidget::NativeDestruct()
{
	Unbind();

	Super::NativeDestruct();
}

void USkillBarWidget::BindToPawn(APawn* Pawn)
{
	// Rebinding (a new pawn after respawn) must not leave the old listeners behind.
	Unbind();

	if (Pawn == nullptr)
	{
		return;
	}

	BoundSkills = Pawn->FindComponentByClass<USkillComponent>();
	BoundMana = Pawn->FindComponentByClass<UManaComponent>();

	if (USkillComponent* Skills = BoundSkills.Get())
	{
		Skills->OnSkillUsed.AddDynamic(this, &USkillBarWidget::HandleSkillUsed);
		Skills->OnPageChanged.AddDynamic(this, &USkillBarWidget::HandlePageChanged);
	}

	if (UManaComponent* Mana = BoundMana.Get())
	{
		Mana->OnManaChanged.AddDynamic(this, &USkillBarWidget::HandleManaChanged);
	}

	RefreshSlots();
}

void USkillBarWidget::Unbind()
{
	if (USkillComponent* Skills = BoundSkills.Get())
	{
		Skills->OnSkillUsed.RemoveDynamic(this, &USkillBarWidget::HandleSkillUsed);
		Skills->OnPageChanged.RemoveDynamic(this, &USkillBarWidget::HandlePageChanged);
	}

	if (UManaComponent* Mana = BoundMana.Get())
	{
		Mana->OnManaChanged.RemoveDynamic(this, &USkillBarWidget::HandleManaChanged);
	}

	BoundSkills = nullptr;
	BoundMana = nullptr;
}

void USkillBarWidget::RefreshSlots()
{
	const USkillComponent* Skills = BoundSkills.Get();
	const int32 Page = Skills != nullptr ? Skills->GetCurrentPage() : 0;

	for (int32 PageSlot = 0; PageSlot < SkillSlots.Num(); ++PageSlot)
	{
		const int32 SlotIndex = Page * USkillComponent::SlotsPerPage + PageSlot;
		const USkillData* Skill = Skills != nullptr ? Skills->GetSkillAt(SlotIndex) : nullptr;

		// This also clears whatever the slot was showing before (skill and cooldown).
		SkillSlots[PageSlot]->SetSkill(Skill);

		// Cooldowns keep running while another page is open, so restore the remaining time.
		if (Skill != nullptr)
		{
			const float Remaining = Skills->GetCooldownRemaining(SlotIndex);
			if (Remaining > 0.f)
			{
				SkillSlots[PageSlot]->StartCooldown(Remaining, Skill->GetCooldown());
			}
		}
	}

	if (const UManaComponent* Mana = BoundMana.Get())
	{
		RefreshAffordability(Mana->GetMana());
	}

	RefreshPageText();
}

void USkillBarWidget::RefreshAffordability(float CurrentMana)
{
	const USkillComponent* Skills = BoundSkills.Get();
	if (Skills == nullptr)
	{
		return;
	}

	const int32 Page = Skills->GetCurrentPage();
	for (int32 PageSlot = 0; PageSlot < SkillSlots.Num(); ++PageSlot)
	{
		const USkillData* Skill = Skills->GetSkillAt(Page * USkillComponent::SlotsPerPage + PageSlot);

		// Empty slots and free skills are never dimmed.
		const bool bAffordable = Skill == nullptr || CurrentMana >= Skill->GetManaCost();
		SkillSlots[PageSlot]->SetAffordable(bAffordable);
	}
}

void USkillBarWidget::RefreshPageText()
{
	const USkillComponent* Skills = BoundSkills.Get();
	if (PageText == nullptr || Skills == nullptr)
	{
		return;
	}

	PageText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), Skills->GetCurrentPage() + 1, USkillComponent::PageCount)));
}

void USkillBarWidget::HandleSkillUsed(int32 SlotIndex)
{
	const USkillComponent* Skills = BoundSkills.Get();
	if (Skills == nullptr)
	{
		return;
	}

	// SlotIndex is absolute; skills are only used from the current page.
	const int32 PageSlot = SlotIndex - Skills->GetCurrentPage() * USkillComponent::SlotsPerPage;
	const USkillData* Skill = Skills->GetSkillAt(SlotIndex);
	if (Skill == nullptr || !SkillSlots.IsValidIndex(PageSlot))
	{
		return;
	}

	SkillSlots[PageSlot]->StartCooldown(Skills->GetCooldownRemaining(SlotIndex), Skill->GetCooldown());
}

void USkillBarWidget::HandlePageChanged(int32 NewPage)
{
	RefreshSlots();
}

void USkillBarWidget::HandleManaChanged(float NewMana, float MaxMana, float Delta)
{
	RefreshAffordability(NewMana);
}