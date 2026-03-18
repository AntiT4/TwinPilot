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
	CameraBoom->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	OrbitCenterArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("OrbitCenterArrow"));
	OrbitCenterArrow->SetupAttachment(SceneRoot);
	OrbitCenterArrow->ArrowColor = FColor::Yellow;
	OrbitCenterArrow->ArrowSize = 1.0f;
	OrbitCenterArrow->SetHiddenInGame(false);

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

	if (OrbitCenterActor != nullptr)
	{
		FVector BoundsOrigin = FVector::ZeroVector;
		FVector BoundsExtent = FVector::ZeroVector;
		OrbitCenterActor->GetActorBounds(true, BoundsOrigin, BoundsExtent);
		OrbitCenterLocation = BoundsOrigin;
	}

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
	SetCenterLocation(BoundsOrigin);
}

void ADigitalTwinOperatorPawn::SetCenterLocation(FVector PivotLocation)
{
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
