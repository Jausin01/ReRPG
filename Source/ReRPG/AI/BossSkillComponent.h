#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BossSkillComponent.generated.h"

class UBossPhaseComponent;
class UHealthComponent;
class USkillData;
class UStunComponent;
struct FBossPhase;

// The wind-up (telegraph) of a skill started. Sound and aura will listen to this.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBossWindUpStarted, USkillData*, Skill, float, Duration);

// The wind-up ended and the effects of the skill were executed.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossSkillReleased, USkillData*, Skill);

// The wind-up was cancelled (stun, death, phase change) and the effects did not run.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossSkillInterrupted, USkillData*, Skill);

// Lets a boss use the skills of its current phase: picks one, waits its wind-up and executes it.
// It uses one timer and the events of other components: no tick.
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class RERPG_API UBossSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBossSkillComponent();

	// Picks a skill of the current phase and starts it. Returns false if there is no skill
	// ready or the boss cannot cast now (stunned, dead, changing phase, already casting).
	UFUNCTION(BlueprintCallable, Category = "Boss Skill")
	bool TryUseSkill();

	// Cancels the skill that is in its wind-up. Does nothing if there is none.
	UFUNCTION(BlueprintCallable, Category = "Boss Skill")
	void CancelCast();

	// True from the start of the wind-up until the effects run or the cast is cancelled.
	UFUNCTION(BlueprintPure, Category = "Boss Skill")
	bool IsCasting() const { return CastingSkill.IsValid(); }

	UPROPERTY(BlueprintAssignable, Category = "Boss Skill")
	FOnBossWindUpStarted OnWindUpStarted;

	UPROPERTY(BlueprintAssignable, Category = "Boss Skill")
	FOnBossSkillReleased OnSkillReleased;

	UPROPERTY(BlueprintAssignable, Category = "Boss Skill")
	FOnBossSkillInterrupted OnSkillInterrupted;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool CanCast() const;
	bool IsReady(const USkillData* Skill, float Now) const;

	USkillData* PickSkill(const FBossPhase& Phase, float Now);
	USkillData* PickRandom(const TArray<TObjectPtr<USkillData>>& Skills, float Now) const;
	USkillData* PickSequential(const TArray<TObjectPtr<USkillData>>& Skills, float Now);

	// Runs the effects of the casting skill when the wind-up ends.
	void ReleaseSkill();

	UFUNCTION()
	void HandleStunStarted(float Duration);

	UFUNCTION()
	void HandleDeath(AActor* Killer);

	UFUNCTION()
	void HandlePhaseChanged(int32 PhaseIndex);

	// World time at which each skill is ready again. Cooldowns need no tick or timers.
	TMap<TWeakObjectPtr<const USkillData>, float> CooldownEndTimes;

	TWeakObjectPtr<USkillData> CastingSkill;
	FTimerHandle WindUpTimer;
	TWeakObjectPtr<UBossPhaseComponent> BossPhase;
	TWeakObjectPtr<UHealthComponent> OwnerHealth;
	TWeakObjectPtr<UStunComponent> OwnerStun;

	// Next skill of the list for the Sequential mode. Restarts when the phase changes.
	int32 SequenceIndex = 0;
};