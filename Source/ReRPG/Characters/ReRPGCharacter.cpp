#include "Characters/ReRPGCharacter.h"

#include "Camera/CameraComponent.h"
#include "Combat/DodgeComponent.h"
#include "Combat/LockOnComponent.h"
#include "Combat/MeleeAttackComponent.h"
#include "Combat/ParryComponent.h"
#include "Combat/StunComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Math/RotationMatrix.h"
#include "Skills/SkillComponent.h"

AReRPGCharacter::AReRPGCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 540.f, 0.f);
	Movement->MaxWalkSpeed = 500.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	MeleeAttack = CreateDefaultSubobject<UMeleeAttackComponent>(TEXT("MeleeAttack"));
	DodgeComponent = CreateDefaultSubobject<UDodgeComponent>(TEXT("DodgeComponent"));
	ParryComponent = CreateDefaultSubobject<UParryComponent>(TEXT("ParryComponent"));
	StunComponent = CreateDefaultSubobject<UStunComponent>(TEXT("StunComponent"));
	LockOnComponent = CreateDefaultSubobject<ULockOnComponent>(TEXT("LockOnComponent"));
	SkillComponent = CreateDefaultSubobject<USkillComponent>(TEXT("SkillComponent"));
}

void AReRPGCharacter::BeginPlay()
{
	Super::BeginPlay();

	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController == nullptr || DefaultMappingContext == nullptr)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (Subsystem != nullptr)
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void AReRPGCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInput == nullptr)
	{
		return;
	}

	if (MoveAction != nullptr)
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AReRPGCharacter::Move);
	}

	if (LookAction != nullptr)
	{
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AReRPGCharacter::Look);
	}

	if (AttackAction != nullptr)
	{
		EnhancedInput->BindAction(AttackAction, ETriggerEvent::Started, this, &AReRPGCharacter::Attack);
	}

	if (DodgeAction != nullptr)
	{
		EnhancedInput->BindAction(DodgeAction, ETriggerEvent::Started, this, &AReRPGCharacter::Dodge);
	}

	if (ParryAction != nullptr)
	{
		EnhancedInput->BindAction(ParryAction, ETriggerEvent::Started, this, &AReRPGCharacter::Parry);
	}

	if (LockOnAction != nullptr)
	{
		EnhancedInput->BindAction(LockOnAction, ETriggerEvent::Started, this, &AReRPGCharacter::ToggleLockOn);
	}

	// Triggered fires once per wheel notch (the value is non-zero for that frame only).
	if (LockOnSwitchAction != nullptr)
	{
		EnhancedInput->BindAction(LockOnSwitchAction, ETriggerEvent::Triggered, this, &AReRPGCharacter::SwitchLockOnTarget);
	}

	// Each skill action is bound with its slot index as payload.
	for (int32 SlotIndex = 0; SlotIndex < SkillActions.Num(); ++SlotIndex)
	{
		if (SkillActions[SlotIndex] != nullptr)
		{
			EnhancedInput->BindAction(SkillActions[SlotIndex], ETriggerEvent::Started, this, &AReRPGCharacter::UseSkill, SlotIndex);
		}
	}

	if (SkillPageAction != nullptr)
	{
		EnhancedInput->BindAction(SkillPageAction, ETriggerEvent::Started, this, &AReRPGCharacter::ToggleSkillPage);
	}
}

void AReRPGCharacter::Move(const FInputActionValue& Value)
{
	if (Controller == nullptr)
	{
		return;
	}

	const FVector2D MoveVector = Value.Get<FVector2D>();
	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);

	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, MoveVector.Y);
	AddMovementInput(Right, MoveVector.X);
}

void AReRPGCharacter::Look(const FInputActionValue& Value)
{
	// While locked, the camera is driven by the target. Release the lock to look around.
	if (LockOnComponent != nullptr && LockOnComponent->IsLockedOn())
	{
		return;
	}

	const FVector2D LookVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
}

void AReRPGCharacter::Attack()
{
	if (MeleeAttack != nullptr)
	{
		MeleeAttack->RequestAttack();
	}
}

void AReRPGCharacter::Dodge()
{
	if (DodgeComponent != nullptr)
	{
		DodgeComponent->RequestDodge();
	}
}

void AReRPGCharacter::Parry()
{
	if (ParryComponent != nullptr)
	{
		ParryComponent->RequestParry();
	}
}

void AReRPGCharacter::ToggleLockOn()
{
	if (LockOnComponent != nullptr)
	{
		LockOnComponent->ToggleLockOn();
	}
}

void AReRPGCharacter::SwitchLockOnTarget(const FInputActionValue& Value)
{
	if (LockOnComponent != nullptr)
	{
		// Wheel up is positive but means "left", so the sign is inverted.
		LockOnComponent->SwitchTarget(-Value.Get<float>());
	}
}

void AReRPGCharacter::UseSkill(int32 PageSlot)
{
	if (SkillComponent != nullptr)
	{
		SkillComponent->TryUseSkill(PageSlot);
	}
}

void AReRPGCharacter::ToggleSkillPage()
{
	if (SkillComponent != nullptr)
	{
		SkillComponent->TogglePage();
	}
}