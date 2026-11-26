#pragma once

#include "CoreMinimal.h"

class AActor;
class UHealthComponent;

namespace CombatTargeting
{
	// Fills OutTargets with the health of every pawn inside the sphere that is hostile to the
	// instigator. The instigator itself, allies and neutral pawns are skipped, and each actor
	// is listed once even if it has several collision pieces.
	void FindHostileTargets(const AActor* Instigator, const FVector& Center, float Radius, TArray<TWeakObjectPtr<UHealthComponent>>& OutTargets);
}