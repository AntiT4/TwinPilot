#include "Controller/DigitalTwinOperatorController.h"

#include "Pawn/DigitalTwinOperatorPawn.h"

#include "Engine/EngineTypes.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogTwinPilotOperatorController, Log, All);

void ADigitalTwinOperatorController::BeginPlay()
{
	Super::BeginPlay();
	ApplyMouseCursorPolicy();
}

void ADigitalTwinOperatorController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent == nullptr)
	{
		return;
	}

	if (bEnableMouseClickSelection && SelectActionName != NAME_None)
	{
		InputComponent->BindAction(SelectActionName, IE_Pressed, this, &ADigitalTwinOperatorController::OnSelectPressed);
	}

	if (RotateActionName != NAME_None)
	{
		InputComponent->BindAction(RotateActionName, IE_Pressed, this, &ADigitalTwinOperatorController::OnRotatePressed);
		InputComponent->BindAction(RotateActionName, IE_Released, this, &ADigitalTwinOperatorController::OnRotateReleased);
	}

	if (MouseXAxisName != NAME_None)
	{
		InputComponent->BindAxis(MouseXAxisName, this, &ADigitalTwinOperatorController::OnMouseX);
	}

	if (MouseYAxisName != NAME_None)
	{
		InputComponent->BindAxis(MouseYAxisName, this, &ADigitalTwinOperatorController::OnMouseY);
	}
}

bool ADigitalTwinOperatorController::SelectActorUnderCursor()
{
	FHitResult Hit;
	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	const bool bHit = GetHitResultUnderCursorByChannel(TraceChannel, true, Hit);
	AActor* HitActor = bHit ? Hit.GetActor() : nullptr;

	SelectedActor = HitActor;

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
		BeginOrbitWithBestPivot();
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

void ADigitalTwinOperatorController::OnSelectPressed()
{
	SelectActorUnderCursor();
}

void ADigitalTwinOperatorController::OnRotatePressed()
{
	SetRotateHeld(true);
}

void ADigitalTwinOperatorController::OnRotateReleased()
{
	SetRotateHeld(false);
}

void ADigitalTwinOperatorController::OnMouseX(float AxisValue)
{
	if (!bRotateHeld || FMath::IsNearlyZero(AxisValue))
	{
		return;
	}

	ADigitalTwinOperatorPawn* OperatorPawn = Cast<ADigitalTwinOperatorPawn>(GetPawn());
	if (OperatorPawn != nullptr)
	{
		OperatorPawn->OrbitByMouseDelta(AxisValue, 0.0f);
	}
}

void ADigitalTwinOperatorController::OnMouseY(float AxisValue)
{
	if (!bRotateHeld || FMath::IsNearlyZero(AxisValue))
	{
		return;
	}

	ADigitalTwinOperatorPawn* OperatorPawn = Cast<ADigitalTwinOperatorPawn>(GetPawn());
	if (OperatorPawn != nullptr)
	{
		OperatorPawn->OrbitByMouseDelta(0.0f, AxisValue);
	}
}

void ADigitalTwinOperatorController::BeginOrbitWithBestPivot()
{
	ADigitalTwinOperatorPawn* OperatorPawn = Cast<ADigitalTwinOperatorPawn>(GetPawn());
	if (OperatorPawn == nullptr)
	{
		return;
	}

	if (SelectedActor != nullptr)
	{
		OperatorPawn->BeginOrbitAroundActor(SelectedActor);
		return;
	}

	if (bUseCursorHitAsOrbitPivotWhenNoSelection)
	{
		FHitResult Hit;
		const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_Visibility);
		if (GetHitResultUnderCursorByChannel(TraceChannel, true, Hit))
		{
			OperatorPawn->BeginOrbitAroundLocation(Hit.ImpactPoint);
			return;
		}
	}

	const FVector FallbackPivot = OperatorPawn->GetActorLocation() + OperatorPawn->GetActorForwardVector() * 1000.0f;
	OperatorPawn->BeginOrbitAroundLocation(FallbackPivot);
}
