#include "AI/EnemyResetComponent.h"

#include "AI/BossPhaseComponent.h"
#include "AI/BossSkillComponent.h"
#include "AI/EnemyAIController.h"
#include "Combat/HealthComponent.h"
#include "Combat/MeleeAttackComponent.h"
#include "Core/EncounterSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UEnemyResetComponent::UEnemyResetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyResetComponent::BeginPlay()
{
	Super::BeginPlay();

	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	SpawnTransform = Owner->GetActorTransform();

	if (UEncounterSubsystem* Encounters = UWorld::GetSubsystem<UEncounterSubsystem>(GetWorld()))
	{
		ResetHandle = Encounters->OnEncountersReset.AddUObject(this, &UEnemyResetComponent::HandleReset);
	}
}

void UEnemyResetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ResetHandle.IsValid())
	{
		if (UEncounterSubsystem* Encounters = UWorld::GetSubsystem<UEncounterSubsystem>(GetWorld()))
		{
			Encounters->OnEncountersReset.Remove(ResetHandle);
		}

		ResetHandle.Reset();
	}

	Super::EndPlay(EndPlayReason);
}

void UEnemyResetComponent::HandleReset()
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	UHealthComponent* Health = Owner->FindComponentByClass<UHealthComponent>();

	// A defeated enemy that stays dead is left as it is: no revive and no AI restart.
	if (bStaysDeadOnceKilled && Health != nullptr && Health->IsDead())
	{
		return;
	}

	// Reset is rare, so the optional components are looked up here instead of being cached.
	// Stop whatever is in progress first, so no timer fires on the revived enemy.
	if (UMeleeAttackComponent* Melee = Owner->FindComponentByClass<UMeleeAttackComponent>())
	{
		Melee->CancelAttack();
	}

	if (UBossSkillComponent* Skills = Owner->FindComponentByClass<UBossSkillComponent>())
	{
		Skills->CancelCast();
	}

	// Bosses go back to the first phase (this also ends a transition in progress).
	if (UBossPhaseComponent* Phase = Owner->FindComponentByClass<UBossPhaseComponent>())
	{
		Phase->ResetPhase();
	}

	// Back to the place where it started.
	Owner->TeleportTo(SpawnTransform.GetLocation(), SpawnTransform.Rotator(), false, true);
	if (const ACharacter* Character = Cast<ACharacter>(Owner))
	{
		Character->GetCharacterMovement()->StopMovementImmediately();
	}

	if (Health != nullptr)
	{
		Health->Revive(100.f);
	}

	// Last: the AI forgets the fight and starts deciding again.
	const APawn* Pawn = Cast<APawn>(Owner);
	if (AEnemyAIController* Controller = Pawn != nullptr ? Cast<AEnemyAIController>(Pawn->GetController()) : nullptr)
	{
		Controller->ResetState();
	}
}