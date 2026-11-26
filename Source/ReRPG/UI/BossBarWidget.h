#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BossBarWidget.generated.h"

class UEnemyInfoComponent;
class UHealthComponent;
class UResourceBarWidget;
class UTextBlock;

// Large boss bar at the bottom of the screen. It follows the boss reported by the encounter
// subsystem and is hidden while no boss is in combat. The layout is built in a Widget Blueprint.
UCLASS()
class RERPG_API UBossBarWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Required: a WBP_BossHealthBar (ResourceBarWidget child) named "HealthBar".
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UResourceBarWidget> HealthBar;

	// Optional: a TextBlock named "BossNameText".
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BossNameText;

private:
	UFUNCTION()
	void HandleActiveBossChanged(UEnemyInfoComponent* Boss);

	UFUNCTION()
	void HandleHealthChanged(float NewHealth, float MaxHealth, float Delta);

	// Stops listening to the current boss's health.
	void UnbindBoss();

	TWeakObjectPtr<UHealthComponent> BoundHealth;
};