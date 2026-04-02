#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/TwinPilotPawnControl.h"
#include "DigitalTwinOperatorPawn.generated.h"

class UCameraComponent;
class UArrowComponent;
class UFloatingPawnMovement;
class UPawnMovementComponent;
class USceneComponent;
class USpringArmComponent;
class AActor;

UCLASS(Blueprintable)
class TWINPILOT_API ADigitalTwinOperatorPawn : public APawn, public ITwinPilotPawnControl
{
	GENERATED_BODY()

public:
	ADigitalTwinOperatorPawn();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Input")
	void ApplyMoveInput(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Input")
	void ApplyVerticalInput(float Up);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Input")
	void SetLookInput(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Input")
	void AddLookInputDelta(float YawDelta, float PitchDelta);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Input")
	void SetSprintEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Settings")
	void SetMoveSpeed(float NewMoveSpeed);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Settings")
	void SetLookSensitivity(float NewLookSensitivity);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Camera")
	void SetLookDistance(float NewLookDistance);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Camera")
	void AddLookDistanceDelta(float LookDistanceDelta, bool bInvertDelta = false);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Camera")
	void FocusAtLocation(FVector TargetWorldLocation, float DesiredDistance = 400.0f);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Camera")
	void GoTowardActor(const AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Camera")
	void SetCenterActor(const AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Camera")
	void SetCenterLocation(FVector PivotLocation);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Camera")
	void BeginOrbit();

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Camera")
	void EndOrbit();

	UFUNCTION(BlueprintPure, Category = "TwinPilot|Camera")
	bool IsOrbiting() const { return bOrbiting; }

	UFUNCTION(BlueprintPure, Category = "TwinPilot|Camera")
	bool IsPanning() const { return bPanning; }

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Input")
	void ResetInput();

	virtual void HandleConfirmedActor_Implementation(const AActor* TargetActor) override;
	virtual void SetOrbitHeld_Implementation(bool bHeld) override;
	virtual void SetPanHeld_Implementation(bool bHeld) override;
	virtual void AddPanInputDelta_Implementation(float ScreenXDelta, float ScreenYDelta) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TwinPilot|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TwinPilot|Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TwinPilot|Components")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TwinPilot|Components")
	TObjectPtr<UArrowComponent> OrbitCenterArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TwinPilot|Components")
	TObjectPtr<UFloatingPawnMovement> MovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Movement", meta = (ClampMin = "50.0"))
	float MoveSpeed = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Movement", meta = (ClampMin = "1.0"))
	float SprintMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Look", meta = (ClampMin = "0.1"))
	float LookSensitivity = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Camera", meta = (ClampMin = "0.0"))
	float LookDistance = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Camera", meta = (ClampMin = "0.0"))
	float MinLookDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Camera", meta = (ClampMin = "0.0"))
	float MaxLookDistance = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Camera", meta = (ClampMin = "0.01"))
	float LookDistanceSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Look")
	bool bInvertLookYaw = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Look")
	bool bInvertLookPitch = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Look", meta = (ClampMin = "-89.0", ClampMax = "0.0"))
	float MinPitch = -80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Look", meta = (ClampMin = "0.0", ClampMax = "89.0"))
	float MaxPitch = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Orbit", meta = (ClampMin = "0.01"))
	float OrbitMouseSensitivity = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Camera", meta = (ClampMin = "0.01"))
	float GoTowardInterpSpeed = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Camera", meta = (ClampMin = "0.0"))
	float GoTowardArrivalTolerance = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Camera", meta = (ClampMin = "0.01"))
	float CameraBoomProbeSize = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Pan", meta = (ClampMin = "0.01"))
	float PanSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinPilot|Orbit")
	TObjectPtr<AActor> OrbitCenterActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinPilot|Orbit")
	FVector OrbitCenterLocation = FVector::ZeroVector;

private:
	float GetCurrentCameraBoomLength() const;
	bool IsCameraBoomCollisionConstrained() const;

	void UpdateMovementSettings() const;
	void UpdateLook();
	void UpdateGoTowardActor(float DeltaSeconds);
	void UpdatePan();
	void UpdateOrbitCenterFromTrackedActor();

	FVector2D MoveInput = FVector2D::ZeroVector;
	FVector2D LookInput = FVector2D::ZeroVector;
	FVector2D PendingPanInput = FVector2D::ZeroVector;
	float VerticalInput = 0.0f;
	FVector GoTowardTargetLocation = FVector::ZeroVector;
	FVector OrbitCenterBaseLocation = FVector::ZeroVector;
	FVector OrbitCenterBoundsExtent = FVector::ZeroVector;
	FVector OrbitCenterOffset = FVector::ZeroVector;
	TObjectPtr<AActor> GoTowardTargetActor;
	bool bIsArrived = true;
	bool bSprintEnabled = false;
	bool bOrbiting = false;
	bool bPanning = false;
};
