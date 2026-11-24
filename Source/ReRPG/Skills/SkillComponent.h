#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillComponent.generated.h"

class UDodgeComponent;
class UHealthComponent;
class UManaComponent;
class USkillData;
class UStunComponent;

// SlotIndex is the absolute slot (0 .. PageCount * SlotsPerPage - 1).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillUsed, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillPageChanged, int32, NewPage);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class RERPG_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	static constexpr int32 SlotsPerPage = 4;
	static constexpr int32 PageCount = 2;

	USkillComponent();

	// Uses the skill in the given slot of the current page (0 .. SlotsPerPage - 1).
	// Returns true if the skill was used.
	UFUNCTION(BlueprintCallable, Category = "Skills")
	bool TryUseSkill(int32 PageSlot);

	// Switches to the next page of skills (wraps around).
	UFUNCTION(BlueprintCallable, Category = "Skills")
	void TogglePage();

	UFUNCTION(BlueprintPure, Category = "Skills")
	int32 GetCurrentPage() const { return CurrentPage; }

	// SlotIndex is absolute (page * SlotsPerPage + slot). Returns null for an empty slot.
	UFUNCTION(BlueprintPure, Category = "Skills")
	USkillData* GetSkillAt(int32 SlotIndex) const;

	// Seconds until the slot is ready again (0 when ready). Used by the skill bar UI.
	UFUNCTION(BlueprintPure, Category = "Skills")
	float GetCooldownRemaining(int32 SlotIndex) const;

	UPROPERTY(BlueprintAssignable, Category = "Skills")
	FOnSkillUsed OnSkillUsed;

	UPROPERTY(BlueprintAssignable, Category = "Skills")
	FOnSkillPageChanged OnPageChanged;

protected:
	virtual void BeginPlay() override;

private:
	bool CanCast() const;

	// One entry per slot: page 0 slots first, then page 1 slots.
	UPROPERTY(EditDefaultsOnly, Category = "Skills")
	TArray<TObjectPtr<USkillData>> Skills;

	// World time at which each slot is ready again. Cooldowns need no tick or timers.
	TArray<float> CooldownEndTimes;

	TWeakObjectPtr<UHealthComponent> OwnerHealth;
	TWeakObjectPtr<UManaComponent> OwnerMana;
	TWeakObjectPtr<UStunComponent> OwnerStun;
	TWeakObjectPtr<UDodgeComponent> OwnerDodge;
	int32 CurrentPage = 0;
};