#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "EnemyHealthBarComponent.generated.h"

class UEnemyInfoComponent;
class UHealthComponent;
class UResourceBarWidget;

// Small health bar floating above a minor enemy. It is only visible while the enemy is in
// combat. The widget class (a ResourceBarWidget child) is picked in the Blueprint.
UCLASS(ClassGroup = (UI), meta = (BlueprintSpawnableComponent))
class RERPG_API UEnemyHealthBarComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UEnemyHealthBarComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleHealthChanged(float NewHealth, float MaxHealth, float Delta);

	UFUNCTION()
	void HandleCombatChanged(bool bInCombat);

	TWeakObjectPtr<UHealthComponent> BoundHealth;
	TWeakObjectPtr<UEnemyInfoComponent> BoundInfo;
	TWeakObjectPtr<UResourceBarWidget> Bar;
};