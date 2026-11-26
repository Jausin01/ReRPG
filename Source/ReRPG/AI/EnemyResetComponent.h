#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyResetComponent.generated.h"

// Lets an enemy or boss start over when the player wakes up after dying: back to the place where
// it started, full health, first phase and out of combat, even if it was dead. Enemies that are
// set to stay dead are the exception: once killed they are not brought back. It only reacts
// to the reset event of the encounter subsystem: no tick.
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class RERPG_API UEnemyResetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyResetComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleReset();

	// When true, this enemy is not revived if it is dead when the player wakes up (a boss that
	// was defeated, for example). If it is still alive it starts over like any other enemy.
	UPROPERTY(EditAnywhere, Category = "Reset")
	bool bStaysDeadOnceKilled = false;

	FDelegateHandle ResetHandle;

	// Where the owner was when the level started.
	FTransform SpawnTransform;
};