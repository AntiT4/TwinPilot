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
		AActor* CurrentConfirmedActor,
		AActor* CurrentSelectedActor)
	{
		if (TargetActor == nullptr)
		{
			return;
		}

		const bool bIsConfirmed = TargetActor == CurrentConfirmedActor;
		const bool bIsSelected = TargetActor == CurrentSelectedActor;

		if (!bIsConfirmed && !bIsSelected)
		{
			SetActorStencilState(TargetActor, false, 0);
			return;
		}

		const int32 StencilValue = bIsConfirmed ? ConfirmedActorStencilValue : SelectedActorStencilValue;
		SetActorStencilState(TargetActor, true, StencilValue);
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

	AActor* PreviousConfirmedActor = ConfirmedActor.Get();
	ConfirmedActor = HitActor;
	RefreshActorStencilByInteractionState(PreviousConfirmedActor, ConfirmedActor.Get(), SelectedActor.Get());
	RefreshActorStencilByInteractionState(ConfirmedActor.Get(), ConfirmedActor.Get(), SelectedActor.Get());

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
	AActor* PreviousSelectedActor = SelectedActor.Get();

	if (HitActor == nullptr)
	{
		return ClearConfirmedActor();
	}

	if (PreviousSelectedActor == HitActor)
	{
		return true;
	}

	SelectedActor = HitActor;
	RefreshActorStencilByInteractionState(PreviousSelectedActor, ConfirmedActor.Get(), SelectedActor.Get());
	RefreshActorStencilByInteractionState(SelectedActor.Get(), ConfirmedActor.Get(), SelectedActor.Get());
	OnActorHovered.Broadcast(SelectedActor);
	return SelectedActor != nullptr;
}

bool ADigitalTwinOperatorController::ClearSelectedActor()
{
	AActor* PreviousSelectedActor = SelectedActor.Get();
	if (PreviousSelectedActor == nullptr)
	{
		return false;
	}

	SelectedActor = nullptr;
	RefreshActorStencilByInteractionState(PreviousSelectedActor, ConfirmedActor.Get(), SelectedActor.Get());
	OnActorHovered.Broadcast(SelectedActor);
	return true;
}

bool ADigitalTwinOperatorController::ClearConfirmedActor()
{
	AActor* PreviousConfirmedActor = ConfirmedActor.Get();
	AActor* PreviousSelectedActor = SelectedActor.Get();
	if (PreviousConfirmedActor == nullptr && PreviousSelectedActor == nullptr)
	{
		return false;
	}

	ConfirmedActor = nullptr;
	SelectedActor = nullptr;

	RefreshActorStencilByInteractionState(PreviousConfirmedActor, ConfirmedActor.Get(), SelectedActor.Get());
	if (PreviousSelectedActor != PreviousConfirmedActor)
	{
		RefreshActorStencilByInteractionState(PreviousSelectedActor, ConfirmedActor.Get(), SelectedActor.Get());
	}

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
