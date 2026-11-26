#include "Core/RespawnComponent.h"

#include "Camera/PlayerCameraManager.h"
#include "Combat/HealthComponent.h"
#include "Combat/ManaComponent.h"
#include "Components/CapsuleComponent.h"
#include "Core/RestPoint.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

URespawnComponent::URespawnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URespawnComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	// Without a rest point the player wakes up where the level started.
	DefaultTransform = Owner->GetActorTransform();

	OwnerHealth = Owner->FindComponentByClass<UHealthComponent>();
	OwnerMana = Owner->FindComponentByClass<UManaComponent>();

	if (UHealthComponent* Health = OwnerHealth.Get())
	{
		Health->OnDeath.AddDynamic(this, &URespawnComponent::HandleDeath);
	}
}

void URespawnComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}

	if (UHealthComponent* Health = OwnerHealth.Get())
	{
		Health->OnDeath.RemoveDynamic(this, &URespawnComponent::HandleDeath);
	}

	Super::EndPlay(EndPlayReason);
}

void URespawnComponent::SetRestPoint(const ARestPoint* NewPoint)
{
	if (RestPoint.Get() == NewPoint)
	{
		return;
	}

	RestPoint = NewPoint;
	UE_LOG(LogTemp, Log, TEXT("Rest point set: %s"), *GetNameSafe(NewPoint));
}

FTransform URespawnComponent::GetRespawnTransform() const
{
	const ARestPoint* Point = RestPoint.Get();
	if (Point == nullptr)
	{
		return DefaultTransform;
	}

	// The rest point is on the floor, so the player is lifted by the capsule to stand on it.
	FTransform Result = Point->GetWakeTransform();
	if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		Result.AddToTranslation(FVector(0.f, 0.f, Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
	}

	return Result;
}

void URespawnComponent::HandleDeath(AActor* Killer)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	UWorld* World = GetWorld();
	if (bRespawning || Pawn == nullptr || World == nullptr)
	{
		return;
	}

	bRespawning = true;

	// A dead player cannot act: this turns off every input action and stops any sliding.
	if (APlayerController* PlayerController = GetPlayerController())
	{
		Pawn->DisableInput(PlayerController);
	}

	if (const ACharacter* Character = Cast<ACharacter>(Pawn))
	{
		Character->GetCharacterMovement()->StopMovementImmediately();
	}

	OnDeathSequenceStarted.Broadcast();

	World->GetTimerManager().SetTimer(SequenceTimer, this, &URespawnComponent::BeginFadeOut, FMath::Max(DeathLingerTime, 0.01f), false);
}

void URespawnComponent::BeginFadeOut()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// Holds the black screen when the fade ends, until the fade in starts.
	const APlayerController* PlayerController = GetPlayerController();
	if (PlayerController != nullptr && PlayerController->PlayerCameraManager != nullptr)
	{
		PlayerController->PlayerCameraManager->StartCameraFade(0.f, 1.f, FadeOutTime, FLinearColor::Black, false, true);
	}

	World->GetTimerManager().SetTimer(SequenceTimer, this, &URespawnComponent::Respawn, FMath::Max(FadeOutTime + BlackHoldTime, 0.01f), false);
}

void URespawnComponent::Respawn()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (Owner == nullptr || World == nullptr)
	{
		return;
	}

	// The screen is black here, so the jump cannot be seen.
	const FTransform Wake = GetRespawnTransform();
	Owner->TeleportTo(Wake.GetLocation(), Wake.Rotator(), false, true);

	APlayerController* PlayerController = GetPlayerController();
	if (PlayerController != nullptr)
	{
		PlayerController->SetControlRotation(Wake.Rotator());
	}

	if (UHealthComponent* Health = OwnerHealth.Get())
	{
		Health->Revive(100.f);
	}

	if (UManaComponent* Mana = OwnerMana.Get())
	{
		Mana->Restore(Mana->GetMaxMana());
	}

	OnRespawned.Broadcast();

	if (PlayerController != nullptr && PlayerController->PlayerCameraManager != nullptr)
	{
		PlayerController->PlayerCameraManager->StartCameraFade(1.f, 0.f, FadeInTime, FLinearColor::Black, false, false);
	}

	World->GetTimerManager().SetTimer(SequenceTimer, this, &URespawnComponent::FinishRespawn, FMath::Max(FadeInTime, 0.01f), false);
}

void URespawnComponent::FinishRespawn()
{
	if (APlayerController* PlayerController = GetPlayerController())
	{
		if (AActor* Owner = GetOwner())
		{
			Owner->EnableInput(PlayerController);
		}
	}

	bRespawning = false;
}

APlayerController* URespawnComponent::GetPlayerController() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	return Pawn != nullptr ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
}