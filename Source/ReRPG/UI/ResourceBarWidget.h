#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ResourceBarWidget.generated.h"

class UProgressBar;
class UTextBlock;

// Reusable bar for any resource (health, mana, ...). It has no game logic: it only displays
// the values it receives. The layout and look are built in a Widget Blueprint child of this class.
UCLASS()
class RERPG_API UResourceBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Updates the fill and the optional "current / max" text. Call it only when the value changes.
	UFUNCTION(BlueprintCallable, Category = "Resource Bar")
	void SetValues(float Current, float Max);

protected:
	// Required: the child Widget Blueprint must contain a ProgressBar named "Bar".
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> Bar;

	// Optional: a TextBlock named "ValueText". If it does not exist, no text is shown.
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ValueText;

private:
	// Last values written to the text, so the text is only rebuilt when the displayed numbers change.
	int32 LastCurrent = INDEX_NONE;
	int32 LastMax = INDEX_NONE;
};