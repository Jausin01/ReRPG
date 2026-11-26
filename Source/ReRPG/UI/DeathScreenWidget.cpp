#include "UI/DeathScreenWidget.h"

#include "Components/Button.h"
#include "Core/RespawnComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

void UDeathScreenWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Hidden until the player dies. The buttons are bound once, when the widget is created.
	SetVisibility(ESlateVisibility::Collapsed);

	ContinueButton->OnClicked.AddDynamic(this, &UDeathScreenWidget::HandleContinueClicked);
	LeaveButton->OnClicked.AddDynamic(this, &UDeathScreenWidget::HandleLeaveClicked);
}

void UDeathScreenWidget::NativeDestruct()
{
	Unbind();

	Super::NativeDestruct();
}

void UDeathScreenWidget::BindToPawn(APawn* Pawn)
{
	Unbind();

	if (Pawn == nullptr)
	{
		return;
	}

	BoundRespawn = Pawn->FindComponentByClass<URespawnComponent>();
	if (URespawnComponent* Respawn = BoundRespawn.Get())
	{
		Respawn->OnChoiceRequested.AddDynamic(this, &UDeathScreenWidget::HandleChoiceRequested);
		Respawn->OnRespawned.AddDynamic(this, &UDeathScreenWidget::HandleRespawned);
	}
}

void UDeathScreenWidget::Unbind()
{
	if (URespawnComponent* Respawn = BoundRespawn.Get())
	{
		Respawn->OnChoiceRequested.RemoveDynamic(this, &UDeathScreenWidget::HandleChoiceRequested);
		Respawn->OnRespawned.RemoveDynamic(this, &UDeathScreenWidget::HandleRespawned);
	}

	BoundRespawn = nullptr;
}

void UDeathScreenWidget::HandleChoiceRequested()
{
	ShowScreen();
}

void UDeathScreenWidget::HandleRespawned()
{
	HideScreen();
}

void UDeathScreenWidget::HandleContinueClicked()
{
	if (URespawnComponent* Respawn = BoundRespawn.Get())
	{
		Respawn->ConfirmContinue();
	}
}

void UDeathScreenWidget::HandleLeaveClicked()
{
	// Closes the game. When there is a main menu this becomes "return to the main menu".
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UDeathScreenWidget::ShowScreen()
{
	SetVisibility(ESlateVisibility::Visible);

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
		PlayerController->SetShowMouseCursor(true);
	}

	// Keyboard and gamepad start on "Continue".
	ContinueButton->SetKeyboardFocus();
}

void UDeathScreenWidget::HideScreen()
{
	SetVisibility(ESlateVisibility::Collapsed);

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
	}
}