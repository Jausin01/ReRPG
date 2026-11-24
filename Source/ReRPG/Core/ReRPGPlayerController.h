#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ReRPGPlayerController.generated.h"

class APawn;
class UPlayerHUDWidget;

// Player controller of the game. It owns the HUD: creates it once and keeps it bound
// to whichever pawn the player currently possesses.
UCLASS()
class RERPG_API AReRPGPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:
	// Widget Blueprint used as the HUD (set in the Blueprint child of this controller).
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPlayerHUDWidget> HUDWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UPlayerHUDWidget> HUD;
};