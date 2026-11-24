#include "Combat/LockOnComponent.h"

#include "Combat/HealthComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"

ULockOnComponent::ULockOnComponent()
{
	// Tick is only enabled while a target is locked.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void ULockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void ULockOnComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Restores the character settings and clears timers and delegates.
	if (bIsLockedOn)
	{
		SetTarget(nullptr);
	}

	Super::EndPlay(EndPlayReason);
}

void ULockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const AActor* Target = LockedTarget.Get();
	const ACharacter* Character = OwnerCharacter.Get();
	if (Target == nullptr || Character == nullptr)
	{
		return;
	}

	AController* Controller = Character->GetController();
	if (Controller == nullptr)
	{
		return;
	}

	// Yaw follows the target, pitch stays fixed so the camera does not tilt wildly.
	const FVector ToTarget = Target->GetActorLocation() - Character->GetActorLocation();
	FRotator Desired = ToTarget.Rotation();
	Desired.Pitch = LockedPitch;
	Desired.Roll = 0.f;

	Controller->SetControlRotation(FMath::RInterpTo(Controller->GetControlRotation(), Desired, DeltaTime, CameraInterpSpeed));
}

void ULockOnComponent::ToggleLockOn()
{
	if (bIsLockedOn)
	{
		SetTarget(nullptr);
		return;
	}

	AActor* Best = FindBestTarget();
	if (Best != nullptr)
	{
		SetTarget(Best);
	}
}

void ULockOnComponent::SwitchTarget(float Direction)
{
	const UWorld* World = GetWorld();
	const AActor* Current = LockedTarget.Get();
	const AActor* Owner = GetOwner();
	if (World == nullptr || Current == nullptr || Owner == nullptr || FMath::IsNearlyZero(Direction))
	{
		return;
	}

	if (World->GetTimeSeconds() < NextSwitchTime)
	{
		return;
	}
	NextSwitchTime = World->GetTimeSeconds() + SwitchCooldown;

	const FVector Origin = Owner->GetActorLocation();
	FVector CurrentDir = Current->GetActorLocation() - Origin;
	CurrentDir.Z = 0.f;
	CurrentDir = CurrentDir.GetSafeNormal();

	TArray<AActor*> Candidates;
	GatherCandidates(Candidates);

	AActor* Best = nullptr;
	float BestAngle = TNumericLimits<float>::Max();

	for (AActor* Candidate : Candidates)
	{
		if (Candidate == Current)
		{
			continue;
		}

		FVector Dir = Candidate->GetActorLocation() - Origin;
		Dir.Z = 0.f;
		Dir = Dir.GetSafeNormal();

		// In Unreal's left-handed space, a positive cross Z means the candidate is to the right.
		const float Side = FVector::CrossProduct(CurrentDir, Dir).Z;
		if (Side * Direction <= 0.f)
		{
			continue;
		}

		const float Angle = FMath::Acos(FMath::Clamp(FVector::DotProduct(CurrentDir, Dir), -1.f, 1.f));
		if (Angle < BestAngle)
		{
			BestAngle = Angle;
			Best = Candidate;
		}
	}

	if (Best != nullptr)
	{
		SetTarget(Best);
	}
}

void ULockOnComponent::SetTarget(AActor* NewTarget)
{
	// Unhook the previous target.
	if (UHealthComponent* OldHealth = TargetHealth.Get())
	{
		OldHealth->OnDeath.RemoveDynamic(this, &ULockOnComponent::HandleTargetDeath);
	}
	TargetHealth = nullptr;

	LockedTarget = NewTarget;

	if (NewTarget != nullptr)
	{
		UHealthComponent* Health = NewTarget->FindComponentByClass<UHealthComponent>();
		TargetHealth = Health;
		if (Health != nullptr)
		{
			Health->OnDeath.AddDynamic(this, &ULockOnComponent::HandleTargetDeath);
		}

		if (!bIsLockedOn)
		{
			BeginLock();
		}
	}
	else if (bIsLockedOn)
	{
		EndLock();
	}

	OnTargetChanged.Broadcast(NewTarget);
}

void ULockOnComponent::BeginLock()
{
	bIsLockedOn = true;

	// Strafe mode: the character faces the camera direction instead of the movement direction.
	if (ACharacter* Character = OwnerCharacter.Get())
	{
		UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
		bPrevOrientToMovement = Movement->bOrientRotationToMovement;
		bPrevUseControllerYaw = Character->bUseControllerRotationYaw;

		Movement->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}

	SetComponentTickEnabled(true);

	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ValidateTimer, this, &ULockOnComponent::ValidateTarget, ValidationInterval, true);
	}
}

void ULockOnComponent::EndLock()
{
	bIsLockedOn = false;

	if (ACharacter* Character = OwnerCharacter.Get())
	{
		UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
		Movement->bOrientRotationToMovement = bPrevOrientToMovement;
		Character->bUseControllerRotationYaw = bPrevUseControllerYaw;
	}

	SetComponentTickEnabled(false);

	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ValidateTimer);
	}
}

AActor* ULockOnComponent::FindBestTarget() const
{
	const ACharacter* Character = OwnerCharacter.Get();
	if (Character == nullptr || Character->GetController() == nullptr)
	{
		return nullptr;
	}

	// Flat (yaw only) camera direction, so looking up or down does not hide targets.
	const FRotator ControlYaw(0.f, Character->GetController()->GetControlRotation().Yaw, 0.f);
	const FVector CameraForward = ControlYaw.Vector();
	const FVector Origin = Character->GetActorLocation();

	TArray<AActor*> Candidates;
	GatherCandidates(Candidates);

	AActor* Best = nullptr;
	float BestAngle = MaxAcquireAngle;

	for (AActor* Candidate : Candidates)
	{
		FVector Dir = Candidate->GetActorLocation() - Origin;
		Dir.Z = 0.f;
		Dir = Dir.GetSafeNormal();

		const float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(CameraForward, Dir), -1.f, 1.f)));
		if (Angle < BestAngle)
		{
			BestAngle = Angle;
			Best = Candidate;
		}
	}

	return Best;
}

void ULockOnComponent::GatherCandidates(TArray<AActor*>& OutCandidates) const
{
	const AActor* Owner = GetOwner();
	const UWorld* World = GetWorld();
	if (Owner == nullptr || World == nullptr)
	{
		return;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LockOnOverlap), false, Owner);
	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(
		Overlaps,
		Owner->GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(MaxRange),
		QueryParams);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		if (Actor == nullptr || OutCandidates.Contains(Actor))
		{
			continue;
		}

		// Only living things with health can be locked.
		const UHealthComponent* Health = Actor->FindComponentByClass<UHealthComponent>();
		if (Health == nullptr || Health->IsDead() || !HasLineOfSight(Actor))
		{
			continue;
		}

		OutCandidates.Add(Actor);
	}
}

bool ULockOnComponent::HasLineOfSight(const AActor* Target) const
{
	const AActor* Owner = GetOwner();
	const UWorld* World = GetWorld();
	if (Owner == nullptr || World == nullptr || Target == nullptr)
	{
		return false;
	}

	const FVector Offset(0.f, 0.f, LineOfSightHeight);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LockOnLineOfSight), false, Owner);
	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(
		Hit,
		Owner->GetActorLocation() + Offset,
		Target->GetActorLocation() + Offset,
		ECC_Visibility,
		QueryParams);

	// Blocked only if something other than the target is in the way.
	return !bHit || Hit.GetActor() == Target;
}

void ULockOnComponent::ValidateTarget()
{
	const AActor* Target = LockedTarget.Get();
	const AActor* Owner = GetOwner();
	if (Target == nullptr || Owner == nullptr)
	{
		SetTarget(nullptr);
		return;
	}

	const UHealthComponent* Health = TargetHealth.Get();
	const float BreakDistance = MaxRange * BreakRangeMultiplier;
	const float DistSquared = FVector::DistSquared(Owner->GetActorLocation(), Target->GetActorLocation());

	if ((Health != nullptr && Health->IsDead()) || DistSquared > FMath::Square(BreakDistance) || !HasLineOfSight(Target))
	{
		SetTarget(nullptr);
	}
}

void ULockOnComponent::HandleTargetDeath(AActor* Killer)
{
	SetTarget(nullptr);
}