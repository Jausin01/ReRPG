#include "Core/ReRPGPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/Pawn.h"
#include "UI/PlayerHudWidget.h"

void AReRPGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Only the local player has a screen, so only the local controller builds the HUD.
	if (!IsLocalController() || HUDWidgetClass == nullptr)
	{
		return;
	}

	HUD = CreateWidget<UPlayerHUDWidget>(this, HUDWidgetClass);
	if (HUD != nullptr)
	{
		HUD->AddToViewport();

		// The pawn may have been possessed before BeginPlay, when there was no HUD yet to bind.
		if (APawn* CurrentPawn = GetPawn())
		{
			HUD->BindToPawn(CurrentPawn);
		}
	}
}

void AReRPGPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Possession after BeginPlay (respawn, new pawn): point the HUD at the new pawn.
	if (HUD != nullptr)
	{
		HUD->BindToPawn(InPawn);
	}
}

void AReRPGPlayerController::OnUnPossess()
{
	if (HUD != nullptr)
	{
		HUD->Unbind();
	}

	Super::OnUnPossess();
}