#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ReRPGPlayerController.generated.h"

class APawn;
class UDeathScreenWidget;
class UPlayerHUDWidget;

// Player controller of the game. It owns the HUD and the death screen: creates them once and
// keeps them bound to whichever pawn the player currently possesses.
UCLASS()
class RERPG_API AReRPGPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:
	void CreateHUD();
	void CreateDeathScreen();

	// Widget Blueprint used as the HUD (set in the Blueprint child of this controller).
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPlayerHUDWidget> HUDWidgetClass;

	// Widget Blueprint of the screen shown after the player dies.
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UDeathScreenWidget> DeathScreenWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UPlayerHUDWidget> HUD;

	UPROPERTY(Transient)
	TObjectPtr<UDeathScreenWidget> DeathScreen;
};