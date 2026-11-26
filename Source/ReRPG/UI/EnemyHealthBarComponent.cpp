#include "UI/EnemyHealthBarComponent.h"

#include "AI/EnemyInfoComponent.h"
#include "Combat/HealthComponent.h"
#include "GameFramework/Actor.h"
#include "UI/ResourceBarWidget.h"

UEnemyHealthBarComponent::UEnemyHealthBarComponent()
{
	// Screen space: the bar always faces the camera and keeps the same size at any distance.
	SetWidgetSpace(EWidgetSpace::Screen);
	SetDrawAtDesiredSize(true);

	// Default height above the actor origin. Adjust it per enemy in the Blueprint.
	SetRelativeLocation(FVector(0.f, 0.f, 120.f));
}

void UEnemyHealthBarComponent::BeginPlay()
{
	Super::BeginPlay();

	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	BoundInfo = Owner->FindComponentByClass<UEnemyInfoComponent>();
	BoundHealth = Owner->FindComponentByClass<UHealthComponent>();

	UEnemyInfoComponent* Info = BoundInfo.Get();
	UHealthComponent* Health = BoundHealth.Get();

	// Hidden until the combat starts. Bosses never use this bar (they get the big one).
	SetVisibility(false);
	if (Info == nullptr || Health == nullptr || Info->GetBarStyle() != EEnemyBarStyle::Minor)
	{
		return;
	}

	// Make sure the widget exists (it is not created on a dedicated server, so the bar can stay null).
	InitWidget();
	Bar = Cast<UResourceBarWidget>(GetUserWidgetObject());

	Health->OnHealthChanged.AddDynamic(this, &UEnemyHealthBarComponent::HandleHealthChanged);
	HandleHealthChanged(Health->GetHealth(), Health->GetMaxHealth(), 0.f);

	Info->OnCombatChanged.AddDynamic(this, &UEnemyHealthBarComponent::HandleCombatChanged);
	HandleCombatChanged(Info->IsInCombat());
}

void UEnemyHealthBarComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UHealthComponent* Health = BoundHealth.Get())
	{
		Health->OnHealthChanged.RemoveDynamic(this, &UEnemyHealthBarComponent::HandleHealthChanged);
	}

	if (UEnemyInfoComponent* Info = BoundInfo.Get())
	{
		Info->OnCombatChanged.RemoveDynamic(this, &UEnemyHealthBarComponent::HandleCombatChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void UEnemyHealthBarComponent::HandleHealthChanged(float NewHealth, float MaxHealth, float Delta)
{
	if (UResourceBarWidget* BarWidget = Bar.Get())
	{
		BarWidget->SetValues(NewHealth, MaxHealth);
	}
}

void UEnemyHealthBarComponent::HandleCombatChanged(bool bInCombat)
{
	SetVisibility(bInCombat);
}