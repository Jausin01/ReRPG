#include "Core/RestPoint.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Core/RespawnComponent.h"

ARestPoint::ARestPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Direction = CreateDefaultSubobject<UArrowComponent>(TEXT("Direction"));
	Direction->SetupAttachment(Root);

	ActivationSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ActivationSphere"));
	ActivationSphere->SetupAttachment(Root);
	ActivationSphere->InitSphereRadius(250.f);
	ActivationSphere->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	ActivationSphere->SetGenerateOverlapEvents(true);
}

void ARestPoint::BeginPlay()
{
	Super::BeginPlay();

	ActivationSphere->OnComponentBeginOverlap.AddDynamic(this, &ARestPoint::HandleBeginOverlap);
}

void ARestPoint::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Activate(OtherActor);
}

void ARestPoint::Activate(AActor* Interactor)
{
	if (Interactor == nullptr)
	{
		return;
	}

	// Only actors that can respawn (the player) care about rest points.
	if (URespawnComponent* Respawn = Interactor->FindComponentByClass<URespawnComponent>())
	{
		Respawn->SetRestPoint(this);
	}
}

FTransform ARestPoint::GetWakeTransform() const
{
	return FTransform(GetActorRotation(), GetActorLocation());
}