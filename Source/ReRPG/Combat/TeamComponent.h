#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TeamComponent.generated.h"

// Who an actor fights for. Party members use the same team as the player.
UENUM(BlueprintType)
enum class ECombatTeam : uint8
{
	// Nobody attacks it and it attacks nobody (city NPCs, props). Also the default.
	Neutral,

	Player,

	Enemy
};

// Marks the team of a pawn. Attacks only hit pawns of a different, non-neutral team.
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class RERPG_API UTeamComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTeamComponent();

	UFUNCTION(BlueprintPure, Category = "Team")
	ECombatTeam GetTeam() const { return Team; }

	// Changes the team at runtime (for example an NPC that turns hostile).
	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetTeam(ECombatTeam NewTeam) { Team = NewTeam; }

	// Team of an actor, or Neutral if it has no TeamComponent.
	static ECombatTeam GetTeamOf(const AActor* Actor);

	// True only when both actors are on a team and the teams are different.
	static bool AreHostile(const AActor* A, const AActor* B);

private:
	UPROPERTY(EditAnywhere, Category = "Team")
	ECombatTeam Team = ECombatTeam::Neutral;
};