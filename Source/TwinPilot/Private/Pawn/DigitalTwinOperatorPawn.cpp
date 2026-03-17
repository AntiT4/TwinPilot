#include "Pawn/DigitalTwinOperatorPawn.h"

#include "Camera/CameraComponent.h"
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
	CameraBoom->TargetArmLength = 0.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

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

void ADigitalTwinOperatorPawn::ApplyLookInput(float Yaw, float Pitch)
{
	LookInput.X = FMath::Clamp(Yaw, -1.0f, 1.0f);
	LookInput.Y = FMath::Clamp(Pitch, -1.0f, 1.0f);
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

void ADigitalTwinOperatorPawn::BeginOrbitAroundActor(const AActor* TargetActor)
{
	if (TargetActor == nullptr)
	{
		return;
	}

	FVector BoundsOrigin = FVector::ZeroVector;
	FVector BoundsExtent = FVector::ZeroVector;
	TargetActor->GetActorBounds(true, BoundsOrigin, BoundsExtent);
	BeginOrbitAroundLocation(BoundsOrigin);
}

void ADigitalTwinOperatorPawn::BeginOrbitAroundLocation(FVector PivotLocation)
{
	InitializeOrbitFromPivot(PivotLocation);
	ApplyOrbitTransform();
}

void ADigitalTwinOperatorPawn::EndOrbit()
{
	bOrbiting = false;
}

void ADigitalTwinOperatorPawn::OrbitByMouseDelta(float MouseDeltaX, float MouseDeltaY)
{
	if (!bOrbiting)
	{
		return;
	}

	OrbitYaw += MouseDeltaX * OrbitMouseSensitivity;

	const float PitchDirection = bInvertLookY ? 1.0f : -1.0f;
	OrbitPitch += MouseDeltaY * OrbitMouseSensitivity * PitchDirection;
	OrbitPitch = FMath::ClampAngle(OrbitPitch, MinPitch, MaxPitch);

	ApplyOrbitTransform();
}

void ADigitalTwinOperatorPawn::ResetInput()
{
	MoveInput = FVector2D::ZeroVector;
	LookInput = FVector2D::ZeroVector;
	VerticalInput = 0.0f;
	bSprintEnabled = false;
	UpdateMovementSettings();
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

void ADigitalTwinOperatorPawn::UpdateLook()
{
	if (Controller == nullptr || bOrbiting)
	{
		return;
	}

	const float AppliedPitch = bInvertLookY ? -LookInput.Y : LookInput.Y;
	AddControllerYawInput(LookInput.X * LookSensitivity);
	AddControllerPitchInput(AppliedPitch * LookSensitivity);

	FRotator ControlRotation = Controller->GetControlRotation();
	ControlRotation.Pitch = FMath::ClampAngle(ControlRotation.Pitch, MinPitch, MaxPitch);
	Controller->SetControlRotation(ControlRotation);
}

void ADigitalTwinOperatorPawn::InitializeOrbitFromPivot(const FVector& PivotLocation)
{
	OrbitPivot = PivotLocation;

	const FVector ToPawn = GetActorLocation() - OrbitPivot;
	const float RawDistance = ToPawn.IsNearlyZero() ? 1000.0f : ToPawn.Size();
	OrbitDistance = FMath::Clamp(RawDistance, OrbitMinDistance, OrbitMaxDistance);

	const FRotator DirectionRotation = ToPawn.IsNearlyZero() ? GetActorRotation() : ToPawn.Rotation();
	OrbitYaw = DirectionRotation.Yaw;
	OrbitPitch = FMath::ClampAngle(DirectionRotation.Pitch, MinPitch, MaxPitch);
	bOrbiting = true;
}

void ADigitalTwinOperatorPawn::ApplyOrbitTransform()
{
	const FRotator OrbitRotation(OrbitPitch, OrbitYaw, 0.0f);
	const FVector NewLocation = OrbitPivot + OrbitRotation.Vector() * OrbitDistance;
	SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);

	if (Controller != nullptr)
	{
		FRotator LookAtRotation = (OrbitPivot - NewLocation).Rotation();
		LookAtRotation.Pitch = FMath::ClampAngle(LookAtRotation.Pitch, MinPitch, MaxPitch);
		Controller->SetControlRotation(LookAtRotation);
	}
}
