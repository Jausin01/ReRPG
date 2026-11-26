#include "Core/ReRPGPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/Pawn.h"
#include "UI/DeathScreenWidget.h"
#include "UI/PlayerHudWidget.h"

void AReRPGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Only the local player has a screen, so only the local controller builds the widgets.
	if (!IsLocalController())
	{
		return;
	}

	CreateHUD();
	CreateDeathScreen();
}

void AReRPGPlayerController::CreateHUD()
{
	if (HUDWidgetClass == nullptr)
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

void AReRPGPlayerController::CreateDeathScreen()
{
	if (DeathScreenWidgetClass == nullptr)
	{
		return;
	}

	DeathScreen = CreateWidget<UDeathScreenWidget>(this, DeathScreenWidgetClass);
	if (DeathScreen != nullptr)
	{
		// A higher Z order keeps it above the HUD.
		DeathScreen->AddToViewport(10);

		if (APawn* CurrentPawn = GetPawn())
		{
			DeathScreen->BindToPawn(CurrentPawn);
		}
	}
}

void AReRPGPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Possession after BeginPlay (respawn, new pawn): point the widgets at the new pawn.
	if (HUD != nullptr)
	{
		HUD->BindToPawn(InPawn);
	}

	if (DeathScreen != nullptr)
	{
		DeathScreen->BindToPawn(InPawn);
	}
}

void AReRPGPlayerController::OnUnPossess()
{
	if (HUD != nullptr)
	{
		HUD->Unbind();
	}

	if (DeathScreen != nullptr)
	{
		DeathScreen->Unbind();
	}

	Super::OnUnPossess();
}