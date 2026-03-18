#include "Controller/DigitalTwinOperatorController.h"

#include "Libraries/DigitalTwinPawnLibrary.h"
#include "Pawn/DigitalTwinOperatorPawn.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogTwinPilotOperatorController, Log, All);

namespace
{
	constexpr int32 SelectedActorStencilValue = 5;
	constexpr int32 HoveredActorStencilValue = 1;

	void SetActorStencilState(AActor* TargetActor, bool bEnabled, int32 StencilValue)
	{
		if (TargetActor == nullptr)
		{
			return;
		}

		TArray<UPrimitiveComponent*> PrimitiveComponents;
		TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents, true);

		for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
		{
			if (PrimitiveComponent == nullptr)
			{
				continue;
			}

			PrimitiveComponent->SetRenderCustomDepth(bEnabled);
			PrimitiveComponent->SetCustomDepthStencilValue(bEnabled ? StencilValue : 0);
		}
	}

	void RefreshActorStencilByInteractionState(
		AActor* TargetActor,
		AActor* CurrentSelectedActor,
		AActor* CurrentHoveredActor)
	{
		if (TargetActor == nullptr)
		{
			return;
		}

		const bool bIsSelected = TargetActor == CurrentSelectedActor;
		const bool bIsHovered = TargetActor == CurrentHoveredActor;

		if (!bIsSelected && !bIsHovered)
		{
			SetActorStencilState(TargetActor, false, 0);
			return;
		}

		const int32 StencilValue = bIsSelected ? SelectedActorStencilValue : HoveredActorStencilValue;
		SetActorStencilState(TargetActor, true, StencilValue);
	}
}

void ADigitalTwinOperatorController::BeginPlay()
{
	Super::BeginPlay();
	ApplyMouseCursorPolicy();
}

bool ADigitalTwinOperatorController::SelectActorUnderCursor()
{
	AActor* HitActor = nullptr;
	UDigitalTwinPawnLibrary::GetActorUnderCursor(this, HitActor);
	AActor* PreviousSelectedActor = SelectedActor.Get();
	SelectedActor = HitActor;
	RefreshActorStencilByInteractionState(PreviousSelectedActor, SelectedActor.Get(), HoveredActor.Get());
	RefreshActorStencilByInteractionState(SelectedActor.Get(), SelectedActor.Get(), HoveredActor.Get());

	if (SelectedActor != nullptr)
	{
		UE_LOG(
			LogTwinPilotOperatorController,
			Log,
			TEXT("Clicked actor: %s | Class: %s"),
			*GetNameSafe(SelectedActor),
			*GetNameSafe(SelectedActor->GetClass()));
	}
	else
	{
		UE_LOG(LogTwinPilotOperatorController, Log, TEXT("Clicked actor: None"));
	}

	OnActorSelected.Broadcast(SelectedActor);
	return SelectedActor != nullptr;
}

bool ADigitalTwinOperatorController::HoverActorUnderCursor()
{
	AActor* HitActor = nullptr;
	UDigitalTwinPawnLibrary::GetActorUnderCursor(this, HitActor);
	AActor* PreviousHoveredActor = HoveredActor.Get();

	if (PreviousHoveredActor == HitActor)
	{
		return HoveredActor != nullptr;
	}

	HoveredActor = HitActor;
	RefreshActorStencilByInteractionState(PreviousHoveredActor, SelectedActor.Get(), HoveredActor.Get());
	RefreshActorStencilByInteractionState(HoveredActor.Get(), SelectedActor.Get(), HoveredActor.Get());
	OnActorHovered.Broadcast(HoveredActor);
	return HoveredActor != nullptr;
}

void ADigitalTwinOperatorController::SetRotateHeld(bool bHeld)
{
	if (bRotateHeld == bHeld)
	{
		return;
	}

	bRotateHeld = bHeld;

	ADigitalTwinOperatorPawn* OperatorPawn = Cast<ADigitalTwinOperatorPawn>(GetPawn());
	if (OperatorPawn == nullptr)
	{
		return;
	}

	if (bRotateHeld)
	{
		OperatorPawn->BeginOrbit();
	}
	else
	{
		OperatorPawn->EndOrbit();
	}
}

void ADigitalTwinOperatorController::ApplyMouseCursorPolicy()
{
	bShowMouseCursor = bAlwaysShowCursor;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}
