#include "UI/SkillSlotWidget.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Skills/SkillData.h"
#include "TimerManager.h"

void USkillSlotWidget::SetSkill(const USkillData* Skill)
{
	// A different skill invalidates the running cooldown; the owner restarts it if needed.
	ClearCooldown();

	if (Skill == nullptr)
	{
		IconImage->SetVisibility(ESlateVisibility::Hidden);

		if (NameText != nullptr)
		{
			NameText->SetText(FText::GetEmpty());
		}
		return;
	}

	if (NameText != nullptr)
	{
		NameText->SetText(Skill->GetDisplayName());
	}

	// The icon is a soft reference: this loads it asynchronously instead of stalling the game thread.
	const TSoftObjectPtr<UTexture2D>& Icon = Skill->GetIcon();
	if (Icon.IsNull())
	{
		IconImage->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		IconImage->SetBrushFromSoftTexture(Icon, false);
		IconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void USkillSlotWidget::SetKeyLabel(const FText& Label)
{
	if (KeyText != nullptr)
	{
		KeyText->SetText(Label);
	}
}

void USkillSlotWidget::StartCooldown(float Remaining, float Total)
{
	const UWorld* World = GetWorld();
	if (World == nullptr || Remaining <= 0.f || Total <= 0.f)
	{
		ClearCooldown();
		return;
	}

	CooldownEndTime = World->GetTimeSeconds() + Remaining;
	CooldownTotal = Total;
	LastTextKey = INDEX_NONE;

	World->GetTimerManager().SetTimer(CooldownTimer, this, &USkillSlotWidget::UpdateCooldown, UpdateInterval, true);
	UpdateCooldown();
}

void USkillSlotWidget::ClearCooldown()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CooldownTimer);
	}

	CooldownEndTime = 0.f;
	LastTextKey = INDEX_NONE;

	CooldownBar->SetPercent(0.f);
	if (CooldownText != nullptr)
	{
		CooldownText->SetText(FText::GetEmpty());
	}
}

void USkillSlotWidget::SetAffordable(bool bNewAffordable)
{
	if (bNewAffordable == bAffordable)
	{
		return;
	}

	bAffordable = bNewAffordable;
	IconImage->SetColorAndOpacity(bAffordable ? FLinearColor::White : FLinearColor(0.35f, 0.35f, 0.35f, 1.f));
}

void USkillSlotWidget::NativeDestruct()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CooldownTimer);
	}

	Super::NativeDestruct();
}

void USkillSlotWidget::UpdateCooldown()
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		ClearCooldown();
		return;
	}

	const float Remaining = CooldownEndTime - World->GetTimeSeconds();
	if (Remaining <= 0.f)
	{
		// Done: stop the timer so a ready slot costs nothing.
		ClearCooldown();
		return;
	}

	CooldownBar->SetPercent(FMath::Clamp(Remaining / CooldownTotal, 0.f, 1.f));

	if (CooldownText != nullptr)
	{
		// Whole seconds from 1s up, one decimal below. Only rebuild the text when the shown value changes.
		const bool bWholeSeconds = Remaining >= 1.f;
		const int32 TextKey = bWholeSeconds ? FMath::CeilToInt(Remaining) : -FMath::CeilToInt(Remaining * 10.f);
		if (TextKey != LastTextKey)
		{
			LastTextKey = TextKey;
			CooldownText->SetText(FText::FromString(bWholeSeconds
				? FString::Printf(TEXT("%d"), FMath::CeilToInt(Remaining))
				: FString::Printf(TEXT("%.1f"), Remaining)));
		}
	}
}