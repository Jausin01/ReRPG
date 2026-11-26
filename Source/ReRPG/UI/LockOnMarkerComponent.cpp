#include "UI/LockOnMarkerComponent.h"

#include "Combat/LockOnComponent.h"
#include "GameFramework/Actor.h"

ULockOnMarkerComponent::ULockOnMarkerComponent()
{
	// Screen space: the marker always faces the camera and keeps the same size at any distance.
	SetWidgetSpace(EWidgetSpace::Screen);
	SetDrawAtDesiredSize(true);
}

void ULockOnMarkerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Hidden until something is locked.
	SetVisibility(false);

	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	ULockOnComponent* LockOn = Owner->FindComponentByClass<ULockOnComponent>();
	BoundLockOn = LockOn;
	if (LockOn != nullptr)
	{
		LockOn->OnTargetChanged.AddDynamic(this, &ULockOnMarkerComponent::HandleTargetChanged);
		HandleTargetChanged(LockOn->GetTarget());
	}
}

void ULockOnMarkerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ULockOnComponent* LockOn = BoundLockOn.Get())
	{
		LockOn->OnTargetChanged.RemoveDynamic(this, &ULockOnMarkerComponent::HandleTargetChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void ULockOnMarkerComponent::HandleTargetChanged(AActor* NewTarget)
{
	USceneComponent* TargetRoot = NewTarget != nullptr ? NewTarget->GetRootComponent() : nullptr;
	if (TargetRoot != nullptr)
	{
		// KeepRelativeTransform: the location set in the Blueprint becomes the offset from the target.
		AttachToComponent(TargetRoot, FAttachmentTransformRules::KeepRelativeTransform);
		SetVisibility(true);
		return;
	}

	SetVisibility(false);

	// Go back to the owner so the marker never keeps pointing at an actor that may go away.
	if (const AActor* Owner = GetOwner())
	{
		if (USceneComponent* OwnerRoot = Owner->GetRootComponent())
		{
			AttachToComponent(OwnerRoot, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}
}