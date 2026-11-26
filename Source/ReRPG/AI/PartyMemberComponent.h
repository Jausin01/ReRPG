#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PartyMemberComponent.generated.h"

class UPartyRoleData;

// Marks a character as a party member and gives it a role. It applies the role to the other
// components when the game starts and lets the ally AI read the role. No tick.
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class RERPG_API UPartyMemberComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPartyMemberComponent();

	// The role of this member, or null if none was assigned.
	const UPartyRoleData* GetRole() const { return Role; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Party")
	TObjectPtr<UPartyRoleData> Role;
};