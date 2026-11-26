#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "LockOnMarkerComponent.generated.h"

class ULockOnComponent;

// Small marker shown on the target locked by the owner's LockOnComponent. It is a single
// screen-space widget that attaches itself to the current target, so the engine moves it with
// the target and nothing runs per frame. The widget class is picked in the Blueprint.
UCLASS(ClassGroup = (UI), meta = (BlueprintSpawnableComponent))
class RERPG_API ULockOnMarkerComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	ULockOnMarkerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleTargetChanged(AActor* NewTarget);

	TWeakObjectPtr<ULockOnComponent> BoundLockOn;
};