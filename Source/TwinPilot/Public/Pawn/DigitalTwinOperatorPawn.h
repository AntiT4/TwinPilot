#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DigitalTwinOperatorPawn.generated.h"

class UCameraComponent;
class UFloatingPawnMovement;
class UPawnMovementComponent;
class USceneComponent;
class USpringArmComponent;

UCLASS(Blueprintable)
class TWINPILOT_API ADigitalTwinOperatorPawn : public APawn
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
	void ApplyLookInput(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Input")
	void SetSprintEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Settings")
	void SetMoveSpeed(float NewMoveSpeed);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Settings")
	void SetLookSensitivity(float NewLookSensitivity);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Camera")
	void FocusAtLocation(FVector TargetWorldLocation, float DesiredDistance = 400.0f);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Input")
	void ResetInput();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TwinPilot|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TwinPilot|Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TwinPilot|Components")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TwinPilot|Components")
	TObjectPtr<UFloatingPawnMovement> MovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Movement", meta = (ClampMin = "50.0"))
	float MoveSpeed = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Movement", meta = (ClampMin = "1.0"))
	float SprintMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Look", meta = (ClampMin = "0.1"))
	float LookSensitivity = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Look")
	bool bInvertLookY = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Look", meta = (ClampMin = "-89.0", ClampMax = "0.0"))
	float MinPitch = -80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinPilot|Look", meta = (ClampMin = "0.0", ClampMax = "89.0"))
	float MaxPitch = 80.0f;

private:
	void UpdateMovementSettings() const;
	void UpdateLook();

	FVector2D MoveInput = FVector2D::ZeroVector;
	FVector2D LookInput = FVector2D::ZeroVector;
	float VerticalInput = 0.0f;
	bool bSprintEnabled = false;
};
