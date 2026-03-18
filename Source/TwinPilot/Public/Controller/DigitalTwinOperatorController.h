#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DigitalTwinOperatorController.generated.h"

class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTwinPilotActorSelectedSignature, AActor*, SelectedActor);

UCLASS(Blueprintable)
class TWINPILOT_API ADigitalTwinOperatorController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Interaction")
	bool SelectActorUnderCursor();

	UFUNCTION(BlueprintPure, Category = "TwinPilot|Interaction")
	AActor* GetSelectedActor() const { return SelectedActor; }

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Camera")
	void SetRotateHeld(bool bHeld);

	UPROPERTY(BlueprintAssignable, Category = "TwinPilot|Interaction")
	FTwinPilotActorSelectedSignature OnActorSelected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinPilot|Mouse")
	bool bAlwaysShowCursor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinPilot|Mouse")
	bool bEnableMouseClickSelection = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinPilot|Camera")
	bool bUseCursorHitAsOrbitPivotWhenNoSelection = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinPilot|Input")
	FName SelectActionName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinPilot|Input")
	FName RotateActionName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinPilot|Input")
	FName MouseXAxisName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinPilot|Input")
	FName MouseYAxisName = NAME_None;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "TwinPilot|Interaction")
	TObjectPtr<AActor> SelectedActor;

private:
	void ApplyMouseCursorPolicy();
	void OnSelectPressed();
	void OnRotatePressed();
	void OnRotateReleased();
	void OnMouseX(float AxisValue);
	void OnMouseY(float AxisValue);
	void BeginOrbitWithBestPivot();

	bool bRotateHeld = false;
};
