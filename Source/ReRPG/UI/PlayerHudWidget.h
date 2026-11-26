#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

class APawn;
class UHealthComponent;
class UManaComponent;
class UResourceBarWidget;
class USkillBarWidget;

// Main HUD of the player. It listens to the pawn's health and mana events and forwards the
// values to the bars, and hands the pawn to the skill bar. The layout (positions, sizes) is
// built in a Widget Blueprint child of this class.
UCLASS()
class RERPG_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Starts listening to the pawn's components and shows their current values.
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void BindToPawn(APawn* Pawn);

	// Stops listening. Safe to call more than once.
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void Unbind();

protected:
	virtual void NativeDestruct() override;

	// Required: the child Widget Blueprint must contain two WBP_ResourceBar widgets with these names.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UResourceBarWidget> HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UResourceBarWidget> ManaBar;

	// Optional: a WBP_SkillBar named "SkillBar".
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USkillBarWidget> SkillBar;

private:
	UFUNCTION()
	void HandleHealthChanged(float NewHealth, float MaxHealth, float Delta);

	UFUNCTION()
	void HandleManaChanged(float NewMana, float MaxMana, float Delta);

	TWeakObjectPtr<UHealthComponent> BoundHealth;
	TWeakObjectPtr<UManaComponent> BoundMana;
};