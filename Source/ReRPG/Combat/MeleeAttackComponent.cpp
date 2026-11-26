#include "Combat/MeleeAttackComponent.h"

#include "Combat/CombatTargeting.h"
#include "Combat/HealthComponent.h"
#include "Combat/StunComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

UMeleeAttackComponent::UMeleeAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	AttackSteps.SetNum(3);
	AttackSteps[0].Damage = 10.f;
	AttackSteps[0].HitDelay = 0.1f;
	AttackSteps[1].Damage = 12.f;
	AttackSteps[1].HitDelay = 0.1f;
	AttackSteps[2].Damage = 20.f;
	AttackSteps[2].HitDelay = 0.2f;
}

void UMeleeAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache the sibling components once. A stun interrupts the current attack.
	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	OwnerHealth = Owner->FindComponentByClass<UHealthComponent>();
	OwnerStun = Owner->FindComponentByClass<UStunComponent>();

	if (UStunComponent* Stun = OwnerStun.Get())
	{
		Stun->OnStunStarted.AddDynamic(this, &UMeleeAttackComponent::HandleStunStarted);
	}
}

void UMeleeAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(HitTimer);
		TimerManager.ClearTimer(AttackEndTimer);
		TimerManager.ClearTimer(ComboResetTimer);
	}

	if (UStunComponent* Stun = OwnerStun.Get())
	{
		Stun->OnStunStarted.RemoveDynamic(this, &UMeleeAttackComponent::HandleStunStarted);
	}

	Super::EndPlay(EndPlayReason);
}

bool UMeleeAttackComponent::CanAttack() const
{
	const UHealthComponent* Health = OwnerHealth.Get();
	if (Health != nullptr && Health->IsDead())
	{
		return false;
	}

	const UStunComponent* Stun = OwnerStun.Get();
	if (Stun != nullptr && Stun->IsStunned())
	{
		return false;
	}

	return true;
}

void UMeleeAttackComponent::RequestAttack()
{
	UWorld* World = GetWorld();
	if (bIsAttacking || World == nullptr || !AttackSteps.IsValidIndex(ComboIndex) || !CanAttack())
	{
		return;
	}

	const FMeleeAttackStep& Step = AttackSteps[ComboIndex];
	bIsAttacking = true;

	FTimerManager& TimerManager = World->GetTimerManager();
	TimerManager.ClearTimer(ComboResetTimer);
	TimerManager.SetTimer(AttackEndTimer, this, &UMeleeAttackComponent::FinishAttack, Step.Duration, false);

	OnAttackStarted.Broadcast(ComboIndex);

	// The hit must land inside the attack, so the delay can never exceed the duration.
	const float HitDelay = FMath::Min(Step.HitDelay, Step.Duration);
	if (HitDelay > 0.f)
	{
#if ENABLE_DRAW_DEBUG
		// Telegraph: shows where the hit is going to land while the windup runs.
		const AActor* Owner = GetOwner();
		if (bDrawDebug && Owner != nullptr)
		{
			const FVector Center = Owner->GetActorLocation() + Owner->GetActorForwardVector() * Step.Range;
			DrawDebugSphere(World, Center, Step.Radius, 16, FColor::Yellow, false, HitDelay);
		}
#endif
		TimerManager.SetTimer(HitTimer, this, &UMeleeAttackComponent::ApplyPendingHit, HitDelay, false);
	}
	else
	{
		PerformHit(Step);
	}
}

void UMeleeAttackComponent::CancelAttack()
{
	if (!bIsAttacking)
	{
		return;
	}

	bIsAttacking = false;
	ComboIndex = 0;

	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(HitTimer);
		TimerManager.ClearTimer(AttackEndTimer);
		TimerManager.ClearTimer(ComboResetTimer);
	}

	OnAttackCanceled.Broadcast();
}

void UMeleeAttackComponent::HandleStunStarted(float Duration)
{
	CancelAttack();
	ResetCombo();
}

void UMeleeAttackComponent::ApplyPendingHit()
{
	if (!bIsAttacking || !AttackSteps.IsValidIndex(ComboIndex))
	{
		return;
	}

	// An owner that died during the windup does not deal damage.
	const UHealthComponent* Health = OwnerHealth.Get();
	if (Health != nullptr && Health->IsDead())
	{
		return;
	}

	PerformHit(AttackSteps[ComboIndex]);
}

void UMeleeAttackComponent::FinishAttack()
{
	bIsAttacking = false;

	UWorld* World = GetWorld();
	if (World == nullptr || !AttackSteps.IsValidIndex(ComboIndex))
	{
		ResetCombo();
		return;
	}

	const float ComboWindow = AttackSteps[ComboIndex].ComboWindow;
	ComboIndex = (ComboIndex + 1) % AttackSteps.Num();

	World->GetTimerManager().SetTimer(ComboResetTimer, this, &UMeleeAttackComponent::ResetCombo, ComboWindow, false);
}

void UMeleeAttackComponent::ResetCombo()
{
	ComboIndex = 0;
}

void UMeleeAttackComponent::PerformHit(const FMeleeAttackStep& Step) const
{
	const AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (Owner == nullptr || World == nullptr)
	{
		return;
	}

	const FVector Center = Owner->GetActorLocation() + Owner->GetActorForwardVector() * Step.Range;

#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		DrawDebugSphere(World, Center, Step.Radius, 16, FColor::Red, false, 0.3f);
	}
#endif

	// Only hostile pawns are hit: allies, neutral pawns and the attacker itself are skipped.
	TArray<TWeakObjectPtr<UHealthComponent>> Targets;
	CombatTargeting::FindHostileTargets(Owner, Center, Step.Radius, Targets);

	for (const TWeakObjectPtr<UHealthComponent>& Target : Targets)
	{
		if (UHealthComponent* Health = Target.Get())
		{
			Health->ApplyDamage(Step.Damage, GetOwner());
		}
	}
}