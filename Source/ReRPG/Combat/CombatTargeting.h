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

	// Fills OutTargets with the health of every pawn inside the sphere that is on the same
	// (non-neutral) team as the instigator. The instigator is listed only if bIncludeInstigator
	// is true. Hostile and neutral pawns are skipped.
	void FindAllyTargets(const AActor* Instigator, const FVector& Center, float Radius, bool bIncludeInstigator, TArray<TWeakObjectPtr<UHealthComponent>>& OutTargets);
}