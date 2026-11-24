#include "Combat/DodgeComponent.h"

#include "Combat/HealthComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

UDodgeComponent::UDodgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDodgeComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache the sibling components once so nothing is searched at dodge time.
	AActor* Owner = GetOwner();
	if (Owner != nullptr)
	{
		OwnerCharacter = Cast<ACharacter>(Owner);
		OwnerHealth = Owner->FindComponentByClass<UHealthComponent>();
	}
}

void UDodgeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DodgeEndTimer);
	}

	// Never leave the owner invulnerable if the component goes away mid-dodge.
	if (bIsDodging)
	{
		FinishDodge();
	}

	Super::EndPlay(EndPlayReason);
}

bool UDodgeComponent::CanDodge() const
{
	const UWorld* World = GetWorld();
	const ACharacter* Character = OwnerCharacter.Get();
	if (World == nullptr || Character == nullptr || bIsDodging)
	{
		return false;
	}

	if (World->GetTimeSeconds() < NextDodgeTime)
	{
		return false;
	}

	const UHealthComponent* Health = OwnerHealth.Get();
	if (Health != nullptr && Health->IsDead())
	{
		return false;
	}

	return Character->GetCharacterMovement()->IsMovingOnGround();
}

bool UDodgeComponent::RequestDodge()
{
	if (!CanDodge())
	{
		return false;
	}

	ACharacter* Character = OwnerCharacter.Get();
	UWorld* World = GetWorld();

	// Dodge toward the movement input, or backwards when the player is standing still.
	FVector Direction = Character->GetLastMovementInputVector();
	Direction.Z = 0.f;
	if (!Direction.Normalize())
	{
		Direction = -Character->GetActorForwardVector();
	}

	bIsDodging = true;
	NextDodgeTime = World->GetTimeSeconds() + Cooldown;

	UHealthComponent* Health = OwnerHealth.Get();
	if (Health != nullptr)
	{
		Health->SetInvulnerable(true);
	}

	// XY override replaces the horizontal velocity, Z is untouched.
	Character->LaunchCharacter(Direction * DodgeSpeed, true, false);

	World->GetTimerManager().SetTimer(DodgeEndTimer, this, &UDodgeComponent::FinishDodge, DodgeDuration, false);

	OnDodgeStarted.Broadcast(Direction);
	return true;
}

void UDodgeComponent::FinishDodge()
{
	bIsDodging = false;

	UHealthComponent* Health = OwnerHealth.Get();
	if (Health != nullptr)
	{
		Health->SetInvulnerable(false);
	}
}