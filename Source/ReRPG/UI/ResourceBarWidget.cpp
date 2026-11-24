#include "UI/ResourceBarWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UResourceBarWidget::SetValues(float Current, float Max)
{
	if (Bar != nullptr)
	{
		const float Percent = Max > 0.f ? FMath::Clamp(Current / Max, 0.f, 1.f) : 0.f;
		Bar->SetPercent(Percent);
	}

	if (ValueText != nullptr)
	{
		// Round up so a tiny amount of health is shown as 1 and never as 0 while still alive.
		const int32 CurrentInt = FMath::CeilToInt(Current);
		const int32 MaxInt = FMath::CeilToInt(Max);

		// Mana regenerates in small steps, so skip the text update when the shown numbers are the same.
		if (CurrentInt != LastCurrent || MaxInt != LastMax)
		{
			LastCurrent = CurrentInt;
			LastMax = MaxInt;
			ValueText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentInt, MaxInt)));
		}
	}
}