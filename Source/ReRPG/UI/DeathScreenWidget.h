#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeathScreenWidget.generated.h"

class APawn;
class UButton;
class URespawnComponent;

// The screen shown after the player dies: a message and two choices, Continue and Leave.
// It stays hidden until the respawn component asks for the choice. The layout, the message
// and the fonts are built in a Widget Blueprint child of this class.
UCLASS()
class RERPG_API UDeathScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Starts listening to the respawn component of the pawn.
	UFUNCTION(BlueprintCallable, Category = "Death Screen")
	void BindToPawn(APawn* Pawn);

	// Stops listening. Safe to call more than once.
	UFUNCTION(BlueprintCallable, Category = "Death Screen")
	void Unbind();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	// Required: the child Widget Blueprint must contain two Buttons with these names.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ContinueButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LeaveButton;

private:
	UFUNCTION()
	void HandleChoiceRequested();

	UFUNCTION()
	void HandleRespawned();

	UFUNCTION()
	void HandleContinueClicked();

	UFUNCTION()
	void HandleLeaveClicked();

	// Shows the screen and gives the input (and the cursor) to the buttons.
	void ShowScreen();

	// Hides the screen and gives the input back to the game.
	void HideScreen();

	TWeakObjectPtr<URespawnComponent> BoundRespawn;
};