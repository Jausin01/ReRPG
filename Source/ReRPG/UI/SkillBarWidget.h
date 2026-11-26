#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillBarWidget.generated.h"

class APawn;
class UManaComponent;
class USkillComponent;
class USkillSlotWidget;
class UTextBlock;

// Skill bar of the player: shows the 4 slots of the current page. It listens to the pawn's skill and
// mana events and tells each slot what to display. The layout is built in a Widget Blueprint.
UCLASS()
class RERPG_API USkillBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Starts listening to the pawn's components and shows the current page.
	UFUNCTION(BlueprintCallable, Category = "Skill Bar")
	void BindToPawn(APawn* Pawn);

	// Stops listening. Safe to call more than once.
	UFUNCTION(BlueprintCallable, Category = "Skill Bar")
	void Unbind();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	// Required: four WBP_SkillSlot widgets with these exact names, one per key (1 to 4).
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USkillSlotWidget> SkillSlot0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USkillSlotWidget> SkillSlot1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USkillSlotWidget> SkillSlot2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USkillSlotWidget> SkillSlot3;

	// Optional: a TextBlock named "PageText" showing "1 / 2".
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PageText;

private:
	// Shows the skills of the current page, restarting the cooldown of slots that are still cooling down.
	void RefreshSlots();

	// Dims the slots whose skill costs more mana than the player has.
	void RefreshAffordability(float CurrentMana);

	void RefreshPageText();

	UFUNCTION()
	void HandleSkillUsed(int32 SlotIndex);

	UFUNCTION()
	void HandlePageChanged(int32 NewPage);

	UFUNCTION()
	void HandleManaChanged(float NewMana, float MaxMana, float Delta);

	// The four slot widgets in key order, collected once for easy looping.
	UPROPERTY(Transient)
	TArray<TObjectPtr<USkillSlotWidget>> SkillSlots;

	TWeakObjectPtr<USkillComponent> BoundSkills;
	TWeakObjectPtr<UManaComponent> BoundMana;
};