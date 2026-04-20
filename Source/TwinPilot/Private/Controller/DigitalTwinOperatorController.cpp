#include "Controller/DigitalTwinOperatorController.h"

#include "Interfaces/TwinPilotPawnControl.h"
#include "Libraries/DigitalTwinPawnLibrary.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogTwinPilotOperatorController, Log, All);

namespace
{
	constexpr int32 ConfirmedActorStencilValue = 5;
	constexpr int32 SelectedActorStencilValue = 1;

	void ExecuteOnControlledPawn(APlayerController* PlayerController, TFunctionRef<void(APawn*)> Callback)
	{
		if (PlayerController == nullptr)
		{
			return;
		}

		APawn* ControlledPawn = PlayerController->GetPawn();
		if (ControlledPawn == nullptr)
		{
			return;
		}

		if (!ControlledPawn->GetClass()->ImplementsInterface(UTwinPilotPawnControl::StaticClass()))
		{
			return;
		}

		Callback(ControlledPawn);
	}

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
		AActor* CurrentConfirmedHighlightedActor,
		AActor* CurrentSelectedHighlightedActor)
	{
		if (TargetActor == nullptr)
		{
			return;
		}

		const bool bIsConfirmed = TargetActor == CurrentConfirmedHighlightedActor;
		const bool bIsSelected = TargetActor == CurrentSelectedHighlightedActor;

		if (!bIsConfirmed && !bIsSelected)
		{
			SetActorStencilState(TargetActor, false, 0);
			return;
		}

		const int32 StencilValue = bIsConfirmed ? ConfirmedActorStencilValue : SelectedActorStencilValue;
		SetActorStencilState(TargetActor, true, StencilValue);
	}

	bool AreInteractionStatesEqual(
		const FTwinPilotInteractionState& LeftState,
		const FTwinPilotInteractionState& RightState)
	{
		return LeftState.SelectedActor == RightState.SelectedActor
			&& LeftState.SelectedHighlightedActor == RightState.SelectedHighlightedActor
			&& LeftState.ConfirmedActor == RightState.ConfirmedActor
			&& LeftState.ConfirmedHighlightedActor == RightState.ConfirmedHighlightedActor;
	}

	void AddUniqueActor(TArray<AActor*>& Actors, AActor* Actor)
	{
		if (Actor != nullptr)
		{
			Actors.AddUnique(Actor);
		}
	}
}

void ADigitalTwinOperatorController::BeginPlay()
{
	Super::BeginPlay();
	ApplyMouseCursorPolicy();
}

bool ADigitalTwinOperatorController::ConfirmActorUnderCursor(AActor*& HitActor)
{
	GetSelectableActorUnderCursor(HitActor);

	if (HitActor == nullptr || HitActor != SelectedActor.Get())
	{
		return false;
	}

	const FTwinPilotInteractionState PreviousState = GetInteractionState();
	ConfirmedActor = HitActor;
	UpdateHighlightedActors();
	RefreshInteractionHighlights(PreviousState);
	NotifyInteractionStateChange(PreviousState);

	if (ConfirmedActor != nullptr)
	{
		UE_LOG(
			LogTwinPilotOperatorController,
			Log,
			TEXT("Confirmed actor: %s | Class: %s"),
			*GetNameSafe(ConfirmedActor),
			*GetNameSafe(ConfirmedActor->GetClass()));
	}
	else
	{
		UE_LOG(LogTwinPilotOperatorController, Log, TEXT("Confirmed actor: None"));
	}

	OnActorSelected.Broadcast(ConfirmedActor);
	ExecuteOnControlledPawn(this, [this](APawn* ControlledPawn)
	{
		ITwinPilotPawnControl::Execute_HandleConfirmedActor(ControlledPawn, ConfirmedActor.Get());
	});

	return ConfirmedActor != nullptr;
}

bool ADigitalTwinOperatorController::SelectActorUnderCursor(AActor*& HitActor)
{
	return ConfirmActorUnderCursor(HitActor);
}

bool ADigitalTwinOperatorController::SelectUnderCursor()
{
	AActor* HitActor = nullptr;
	GetSelectableActorUnderCursor(HitActor);

	if (HitActor == nullptr)
	{
		return ClearConfirmedActor();
	}

	if (SelectedActor == HitActor)
	{
		return true;
	}

	const FTwinPilotInteractionState PreviousState = GetInteractionState();
	SelectedActor = HitActor;
	UpdateHighlightedActors();
	RefreshInteractionHighlights(PreviousState);
	NotifyInteractionStateChange(PreviousState);
	OnActorHovered.Broadcast(SelectedActor);
	return SelectedActor != nullptr;
}

bool ADigitalTwinOperatorController::ClearSelectedActor()
{
	if (SelectedActor == nullptr)
	{
		return false;
	}

	const FTwinPilotInteractionState PreviousState = GetInteractionState();
	SelectedActor = nullptr;
	UpdateHighlightedActors();
	RefreshInteractionHighlights(PreviousState);
	NotifyInteractionStateChange(PreviousState);
	OnActorHovered.Broadcast(SelectedActor);
	return true;
}

bool ADigitalTwinOperatorController::ClearConfirmedActor()
{
	if (ConfirmedActor == nullptr && SelectedActor == nullptr)
	{
		return false;
	}

	const FTwinPilotInteractionState PreviousState = GetInteractionState();
	ConfirmedActor = nullptr;
	SelectedActor = nullptr;
	UpdateHighlightedActors();
	RefreshInteractionHighlights(PreviousState);
	NotifyInteractionStateChange(PreviousState);

	OnActorHovered.Broadcast(SelectedActor);
	OnActorSelected.Broadcast(ConfirmedActor);
	ExecuteOnControlledPawn(this, [](APawn* ControlledPawn)
	{
		ITwinPilotPawnControl::Execute_HandleConfirmedActor(ControlledPawn, nullptr);
	});

	return true;
}

bool ADigitalTwinOperatorController::PreselectActorUnderCursor()
{
	return SelectUnderCursor();
}

bool ADigitalTwinOperatorController::HoverActorUnderCursor()
{
	return SelectUnderCursor();
}

void ADigitalTwinOperatorController::SetRotateHeld(bool bHeld)
{
	if (bRotateHeld == bHeld)
	{
		return;
	}

	bRotateHeld = bHeld;
	ExecuteOnControlledPawn(this, [bHeld](APawn* ControlledPawn)
	{
		ITwinPilotPawnControl::Execute_SetOrbitHeld(ControlledPawn, bHeld);
	});
}

void ADigitalTwinOperatorController::SetPanHeld(bool bHeld)
{
	ExecuteOnControlledPawn(this, [bHeld](APawn* ControlledPawn)
	{
		ITwinPilotPawnControl::Execute_SetPanHeld(ControlledPawn, bHeld);
	});
}

void ADigitalTwinOperatorController::AddPanInputDelta(float ScreenXDelta, float ScreenYDelta)
{
	ExecuteOnControlledPawn(this, [ScreenXDelta, ScreenYDelta](APawn* ControlledPawn)
	{
		ITwinPilotPawnControl::Execute_AddPanInputDelta(
			ControlledPawn,
			ScreenXDelta,
			ScreenYDelta);
	});
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

bool ADigitalTwinOperatorController::GetSelectableActorUnderCursor(AActor*& HitActor) const
{
	HitActor = nullptr;
	UDigitalTwinPawnLibrary::GetActorUnderCursor(const_cast<ADigitalTwinOperatorController*>(this), HitActor);

	if (ShouldIgnoreActorForSelection(HitActor))
	{
		UE_LOG(
			LogTwinPilotOperatorController,
			Verbose,
			TEXT("Ignored actor for selection due to tag '%s': %s"),
			*SelectionBlockedTag.ToString(),
			*GetNameSafe(HitActor));
		HitActor = nullptr;
		return false;
	}

	return HitActor != nullptr;
}

bool ADigitalTwinOperatorController::ShouldIgnoreActorForSelection(const AActor* Actor) const
{
	return Actor != nullptr
		&& !SelectionBlockedTag.IsNone()
		&& Actor->ActorHasTag(SelectionBlockedTag);
}

FTwinPilotInteractionState ADigitalTwinOperatorController::GetInteractionState() const
{
	FTwinPilotInteractionState InteractionState;
	InteractionState.SelectedActor = SelectedActor;
	InteractionState.SelectedHighlightedActor = SelectedHighlightedActor;
	InteractionState.ConfirmedActor = ConfirmedActor;
	InteractionState.ConfirmedHighlightedActor = ConfirmedHighlightedActor;
	return InteractionState;
}

void ADigitalTwinOperatorController::UpdateHighlightedActors()
{
	ConfirmedHighlightedActor = ConfirmedActor;
	SelectedHighlightedActor = SelectedActor != ConfirmedActor ? SelectedActor.Get() : nullptr;
}

void ADigitalTwinOperatorController::RefreshInteractionHighlights(const FTwinPilotInteractionState& PreviousState)
{
	const FTwinPilotInteractionState CurrentState = GetInteractionState();
	TArray<AActor*> AffectedActors;
	AddUniqueActor(AffectedActors, PreviousState.SelectedHighlightedActor.Get());
	AddUniqueActor(AffectedActors, PreviousState.ConfirmedHighlightedActor.Get());
	AddUniqueActor(AffectedActors, CurrentState.SelectedHighlightedActor.Get());
	AddUniqueActor(AffectedActors, CurrentState.ConfirmedHighlightedActor.Get());

	for (AActor* AffectedActor : AffectedActors)
	{
		RefreshActorStencilByInteractionState(
			AffectedActor,
			CurrentState.ConfirmedHighlightedActor.Get(),
			CurrentState.SelectedHighlightedActor.Get());
	}
}

void ADigitalTwinOperatorController::NotifyInteractionStateChange(const FTwinPilotInteractionState& PreviousState)
{
	const FTwinPilotInteractionState CurrentState = GetInteractionState();
	if (AreInteractionStatesEqual(PreviousState, CurrentState))
	{
		return;
	}

	LastInteractionStateChange.PreviousState = PreviousState;
	LastInteractionStateChange.CurrentState = CurrentState;
	LastInteractionStateChange.NewlyConfirmedActor =
		PreviousState.ConfirmedActor != CurrentState.ConfirmedActor ? CurrentState.ConfirmedActor.Get() : nullptr;
	LastInteractionStateChange.ReleasedConfirmedActor =
		PreviousState.ConfirmedActor != CurrentState.ConfirmedActor ? PreviousState.ConfirmedActor.Get() : nullptr;

	OnInteractionStateChanged.Broadcast(LastInteractionStateChange);
}
