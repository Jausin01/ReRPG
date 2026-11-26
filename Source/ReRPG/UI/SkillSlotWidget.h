#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillSlotWidget.generated.h"

class UImage;
class UProgressBar;
class USkillData;
class UTextBlock;

// One slot of the skill bar. It only displays what it is told: the skill, the key label,
// a cooldown overlay and whether the skill is affordable. The layout is built in a Widget Blueprint.
UCLASS()
class RERPG_API USkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Shows the skill in this slot (null = empty slot). Clears any running cooldown.
	void SetSkill(const USkillData* Skill);

	UFUNCTION(BlueprintCallable, Category = "Skill Slot")
	void SetKeyLabel(const FText& Label);

	// Starts (or restarts) the cooldown overlay. Values are in seconds; Remaining <= 0 clears it.
	UFUNCTION(BlueprintCallable, Category = "Skill Slot")
	void StartCooldown(float Remaining, float Total);

	UFUNCTION(BlueprintCallable, Category = "Skill Slot")
	void ClearCooldown();

	// Dims the icon when the player cannot pay the mana cost.
	UFUNCTION(BlueprintCallable, Category = "Skill Slot")
	void SetAffordable(bool bNewAffordable);

protected:
	virtual void NativeDestruct() override;

	// Required widgets of the child Widget Blueprint.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IconImage;

	// Dark overlay that shrinks while the skill is on cooldown.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> CooldownBar;

	// Optional widgets: they are simply skipped when the Widget Blueprint does not have them.
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CooldownText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> KeyText;

private:
	void UpdateCooldown();

	// Time between overlay updates while a cooldown is running.
	static constexpr float UpdateInterval = 0.05f;

	FTimerHandle CooldownTimer;
	float CooldownEndTime = 0.f;
	float CooldownTotal = 0.f;

	// Identifies the text currently shown, so it is only rebuilt when it changes.
	int32 LastTextKey = INDEX_NONE;

	bool bAffordable = true;
};