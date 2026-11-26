#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RestPoint.generated.h"

class UArrowComponent;
class USphereComponent;
class USceneComponent;

// A place where the player rests (a bed). After dying, the player wakes up at the last rest
// point used. Place it on the floor: the actor location is the floor and its yaw is where the
// player looks when waking up. Make a Blueprint child to add the bed mesh.
UCLASS(Blueprintable)
class RERPG_API ARestPoint : public AActor
{
	GENERATED_BODY()

public:
	ARestPoint();

	// Makes this the place where the interactor wakes up after dying.
	// The future interaction with the bed (and the save) will call this.
	UFUNCTION(BlueprintCallable, Category = "Rest Point")
	void Activate(AActor* Interactor);

	// Floor location and facing of the wake up.
	UFUNCTION(BlueprintPure, Category = "Rest Point")
	FTransform GetWakeTransform() const;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, Category = "Rest Point")
	TObjectPtr<USceneComponent> Root;

	// Temporary: walking close activates the point. It goes away when the interaction exists.
	UPROPERTY(VisibleAnywhere, Category = "Rest Point")
	TObjectPtr<USphereComponent> ActivationSphere;

	// Shows the facing of the wake up in the editor.
	UPROPERTY(VisibleAnywhere, Category = "Rest Point")
	TObjectPtr<UArrowComponent> Direction;
};