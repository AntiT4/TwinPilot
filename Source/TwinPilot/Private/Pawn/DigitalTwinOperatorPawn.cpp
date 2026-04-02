#include "Pawn/DigitalTwinOperatorPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Math/RotationMatrix.h"

ADigitalTwinOperatorPawn::ADigitalTwinOperatorPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(SceneRoot);
	CameraBoom->TargetArmLength = LookDistance;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = true;
	CameraBoom->ProbeSize = CameraBoomProbeSize;
	CameraBoom->ProbeChannel = ECC_Camera;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	OrbitCenterArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("OrbitCenterArrow"));
	OrbitCenterArrow->SetupAttachment(SceneRoot);
	OrbitCenterArrow->ArrowColor = FColor::Yellow;
	OrbitCenterArrow->ArrowSize = 1.0f;

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovement"));
	MovementComponent->Acceleration = 8000.0f;
	MovementComponent->Deceleration = 12000.0f;
	MovementComponent->UpdatedComponent = SceneRoot;

	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
}

void ADigitalTwinOperatorPawn::BeginPlay()
{
	Super::BeginPlay();
	OrbitCenterLocation = GetActorLocation();
	OrbitCenterBaseLocation = OrbitCenterLocation;
	if (OrbitCenterArrow != nullptr)
	{
		OrbitCenterArrow->SetWorldLocation(OrbitCenterLocation);
	}
	SetLookDistance(LookDistance);
	UpdateMovementSettings();
}

void ADigitalTwinOperatorPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (Controller != nullptr)
	{
		const FRotator ControlRotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MoveInput.Y);
		AddMovementInput(RightDirection, MoveInput.X);
		AddMovementInput(FVector::UpVector, VerticalInput);
	}

	UpdateOrbitCenterFromTrackedActor();
	UpdatePan();
	UpdateGoTowardActor(DeltaSeconds);

	if (OrbitCenterArrow != nullptr)
	{
		OrbitCenterArrow->SetWorldLocation(OrbitCenterLocation);
	}

	UpdateLook();
	UpdateMovementSettings();
}

void ADigitalTwinOperatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UPawnMovementComponent* ADigitalTwinOperatorPawn::GetMovementComponent() const
{
	return MovementComponent;
}

void ADigitalTwinOperatorPawn::ApplyMoveInput(float Right, float Forward)
{
	MoveInput.X = FMath::Clamp(Right, -1.0f, 1.0f);
	MoveInput.Y = FMath::Clamp(Forward, -1.0f, 1.0f);
}

void ADigitalTwinOperatorPawn::ApplyVerticalInput(float Up)
{
	VerticalInput = FMath::Clamp(Up, -1.0f, 1.0f);
}

void ADigitalTwinOperatorPawn::SetLookInput(float Yaw, float Pitch)
{
	const float AppliedYaw = bInvertLookYaw ? -Yaw : Yaw;
	const float AppliedPitch = bInvertLookPitch ? -Pitch : Pitch;
	LookInput.X = FMath::Clamp(AppliedYaw, -1.0f, 1.0f);
	LookInput.Y = FMath::Clamp(AppliedPitch, -1.0f, 1.0f);
}

void ADigitalTwinOperatorPawn::AddLookInputDelta(float YawDelta, float PitchDelta)
{
	if (!bOrbiting)
	{
		return;
	}

	const float AppliedYaw = bInvertLookYaw ? -YawDelta : YawDelta;
	const float AppliedPitch = bInvertLookPitch ? -PitchDelta : PitchDelta;
	LookInput.X += AppliedYaw;
	LookInput.Y += AppliedPitch;
}

void ADigitalTwinOperatorPawn::SetSprintEnabled(bool bEnabled)
{
	bSprintEnabled = bEnabled;
	UpdateMovementSettings();
}

void ADigitalTwinOperatorPawn::SetMoveSpeed(float NewMoveSpeed)
{
	MoveSpeed = FMath::Max(50.0f, NewMoveSpeed);
	UpdateMovementSettings();
}

void ADigitalTwinOperatorPawn::SetLookSensitivity(float NewLookSensitivity)
{
	LookSensitivity = FMath::Max(0.1f, NewLookSensitivity);
}

void ADigitalTwinOperatorPawn::SetLookDistance(float NewLookDistance)
{
	const float SafeMinDistance = FMath::Max(0.0f, MinLookDistance);
	const float SafeMaxDistance = FMath::Max(SafeMinDistance, MaxLookDistance);
	LookDistance = FMath::Clamp(NewLookDistance, SafeMinDistance, SafeMaxDistance);

	if (CameraBoom != nullptr)
	{
		CameraBoom->TargetArmLength = LookDistance;
	}
}

void ADigitalTwinOperatorPawn::AddLookDistanceDelta(float LookDistanceDelta, bool bInvertDelta)
{
	const float EffectiveDelta = bInvertDelta ? -LookDistanceDelta : LookDistanceDelta;
	SetLookDistance(LookDistance + EffectiveDelta * LookDistanceSensitivity);
}

void ADigitalTwinOperatorPawn::FocusAtLocation(FVector TargetWorldLocation, float DesiredDistance)
{
	if (Controller == nullptr)
	{
		return;
	}

	const FVector ToTarget = TargetWorldLocation - GetActorLocation();
	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	FRotator DesiredRotation = ToTarget.Rotation();
	DesiredRotation.Pitch = FMath::ClampAngle(DesiredRotation.Pitch, MinPitch, MaxPitch);
	Controller->SetControlRotation(DesiredRotation);

	if (DesiredDistance > 0.0f)
	{
		const FVector NewLocation = TargetWorldLocation - DesiredRotation.Vector() * DesiredDistance;
		SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void ADigitalTwinOperatorPawn::GoTowardActor(const AActor* TargetActor)
{
	if (TargetActor == nullptr)
	{
		GoTowardTargetActor = nullptr;
		bIsArrived = true;
		return;
	}

	FVector BoundsOrigin = FVector::ZeroVector;
	FVector BoundsExtent = FVector::ZeroVector;
	TargetActor->GetActorBounds(true, BoundsOrigin, BoundsExtent);

	GoTowardTargetActor = const_cast<AActor*>(TargetActor);
	GoTowardTargetLocation = BoundsOrigin;
	OrbitCenterOffset = FVector::ZeroVector;
	bIsArrived = false;
	SetCenterActor(TargetActor);
}

void ADigitalTwinOperatorPawn::SetCenterActor(const AActor* TargetActor)
{
	if (TargetActor == nullptr)
	{
		return;
	}

	FVector BoundsOrigin = FVector::ZeroVector;
	FVector BoundsExtent = FVector::ZeroVector;
	TargetActor->GetActorBounds(true, BoundsOrigin, BoundsExtent);
	OrbitCenterActor = const_cast<AActor*>(TargetActor);
	OrbitCenterBaseLocation = BoundsOrigin;
	OrbitCenterBoundsExtent = BoundsExtent;
	OrbitCenterOffset = FVector::ZeroVector;
	OrbitCenterLocation = OrbitCenterBaseLocation;
}

void ADigitalTwinOperatorPawn::SetCenterLocation(FVector PivotLocation)
{
	OrbitCenterActor = nullptr;
	OrbitCenterBaseLocation = PivotLocation;
	OrbitCenterBoundsExtent = FVector::ZeroVector;
	OrbitCenterOffset = FVector::ZeroVector;
	OrbitCenterLocation = PivotLocation;
}

void ADigitalTwinOperatorPawn::BeginOrbit()
{
	bOrbiting = true;
}

void ADigitalTwinOperatorPawn::EndOrbit()
{
	bOrbiting = false;
}

void ADigitalTwinOperatorPawn::ResetInput()
{
	MoveInput = FVector2D::ZeroVector;
	LookInput = FVector2D::ZeroVector;
	PendingPanInput = FVector2D::ZeroVector;
	VerticalInput = 0.0f;
	bSprintEnabled = false;
	UpdateMovementSettings();
}

void ADigitalTwinOperatorPawn::HandleConfirmedActor_Implementation(const AActor* TargetActor)
{
	GoTowardActor(TargetActor);
}

void ADigitalTwinOperatorPawn::SetOrbitHeld_Implementation(bool bHeld)
{
	if (bHeld)
	{
		BeginOrbit();
		return;
	}

	EndOrbit();
}

void ADigitalTwinOperatorPawn::SetPanHeld_Implementation(bool bHeld)
{
	bPanning = bHeld;
	PendingPanInput = FVector2D::ZeroVector;

	if (bPanning)
	{
		bIsArrived = true;
	}
}

void ADigitalTwinOperatorPawn::AddPanInputDelta_Implementation(float ScreenXDelta, float ScreenYDelta)
{
	if (!bPanning || OrbitCenterActor == nullptr)
	{
		return;
	}

	PendingPanInput += FVector2D(ScreenXDelta, ScreenYDelta);
}

void ADigitalTwinOperatorPawn::UpdateMovementSettings() const
{
	if (MovementComponent == nullptr)
	{
		return;
	}

	const float SpeedMultiplier = bSprintEnabled ? SprintMultiplier : 1.0f;
	MovementComponent->MaxSpeed = MoveSpeed * SpeedMultiplier;
}

void ADigitalTwinOperatorPawn::UpdatePan()
{
	if (!bPanning || OrbitCenterActor == nullptr || PendingPanInput.IsNearlyZero())
	{
		return;
	}

	const FRotator ControlRotation = Controller != nullptr ? Controller->GetControlRotation() : GetActorRotation();
	FVector PlaneRight = FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::Y);
	PlaneRight.Z = 0.0f;
	PlaneRight.Normalize();
	const float VerticalPanScale = FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::X).Size2D();

	const FVector RequestedDelta =
		(PlaneRight * PendingPanInput.X + FVector::UpVector * (PendingPanInput.Y * VerticalPanScale)) * PanSensitivity;
	FVector RequestedOffset = OrbitCenterOffset + RequestedDelta;
	RequestedOffset.X = FMath::Clamp(RequestedOffset.X, -OrbitCenterBoundsExtent.X, OrbitCenterBoundsExtent.X);
	RequestedOffset.Y = FMath::Clamp(RequestedOffset.Y, -OrbitCenterBoundsExtent.Y, OrbitCenterBoundsExtent.Y);
	RequestedOffset.Z = FMath::Clamp(RequestedOffset.Z, -OrbitCenterBoundsExtent.Z, OrbitCenterBoundsExtent.Z);

	const FVector AppliedDelta = RequestedOffset - OrbitCenterOffset;
	OrbitCenterOffset = RequestedOffset;
	OrbitCenterLocation = OrbitCenterBaseLocation + OrbitCenterOffset;

	if (!AppliedDelta.IsNearlyZero())
	{
		SetActorLocation(GetActorLocation() + AppliedDelta, false, nullptr, ETeleportType::TeleportPhysics);
	}

	PendingPanInput = FVector2D::ZeroVector;
}

void ADigitalTwinOperatorPawn::UpdateGoTowardActor(float DeltaSeconds)
{
	if (bIsArrived)
	{
		return;
	}

	if (GoTowardTargetActor != nullptr)
	{
		FVector BoundsOrigin = FVector::ZeroVector;
		FVector BoundsExtent = FVector::ZeroVector;
		GoTowardTargetActor->GetActorBounds(true, BoundsOrigin, BoundsExtent);
		GoTowardTargetLocation = BoundsOrigin;
	}

	const FVector CurrentLocation = GetActorLocation();
	const float ArrivalToleranceSq = FMath::Square(FMath::Max(0.0f, GoTowardArrivalTolerance));

	if (FVector::DistSquared(CurrentLocation, GoTowardTargetLocation) <= ArrivalToleranceSq)
	{
		SetActorLocation(GoTowardTargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		bIsArrived = true;
		return;
	}

	const FVector NewLocation =
		FMath::VInterpTo(CurrentLocation, GoTowardTargetLocation, DeltaSeconds, FMath::Max(0.01f, GoTowardInterpSpeed));
	SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
}

void ADigitalTwinOperatorPawn::UpdateOrbitCenterFromTrackedActor()
{
	if (OrbitCenterActor == nullptr)
	{
		return;
	}

	OrbitCenterOffset.X = FMath::Clamp(OrbitCenterOffset.X, -OrbitCenterBoundsExtent.X, OrbitCenterBoundsExtent.X);
	OrbitCenterOffset.Y = FMath::Clamp(OrbitCenterOffset.Y, -OrbitCenterBoundsExtent.Y, OrbitCenterBoundsExtent.Y);
	OrbitCenterOffset.Z = FMath::Clamp(OrbitCenterOffset.Z, -OrbitCenterBoundsExtent.Z, OrbitCenterBoundsExtent.Z);
	OrbitCenterLocation = OrbitCenterBaseLocation + OrbitCenterOffset;
}

void ADigitalTwinOperatorPawn::UpdateLook()
{
	if (Controller == nullptr)
	{
		return;
	}

	AddControllerYawInput(LookInput.X * LookSensitivity);
	AddControllerPitchInput(LookInput.Y * LookSensitivity);

	FRotator ControlRotation = Controller->GetControlRotation();
	ControlRotation.Pitch = FMath::ClampAngle(ControlRotation.Pitch, MinPitch, MaxPitch);
	Controller->SetControlRotation(ControlRotation);

	if (bOrbiting)
	{
		LookInput = FVector2D::ZeroVector;
	}
}
