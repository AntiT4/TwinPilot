#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DigitalTwinOperatorController.generated.h"

class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTwinPilotActorSelectedSignature, AActor*, SelectedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTwinPilotActorHoveredSignature, AActor*, HoveredActor);

UCLASS(Blueprintable)
class TWINPILOT_API ADigitalTwinOperatorController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Interaction")
	bool SelectActorUnderCursor();

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Interaction")
	bool HoverActorUnderCursor();

	UFUNCTION(BlueprintPure, Category = "TwinPilot|Interaction")
	AActor* GetSelectedActor() const { return SelectedActor; }

	UFUNCTION(BlueprintPure, Category = "TwinPilot|Interaction")
	AActor* GetHoveredActor() const { return HoveredActor; }

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Camera")
	void SetRotateHeld(bool bHeld);

	UPROPERTY(BlueprintAssignable, Category = "TwinPilot|Interaction")
	FTwinPilotActorSelectedSignature OnActorSelected;

	UPROPERTY(BlueprintAssignable, Category = "TwinPilot|Interaction")
	FTwinPilotActorHoveredSignature OnActorHovered;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinPilot|Mouse")
	bool bAlwaysShowCursor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinPilot|Mouse")
	bool bEnableMouseClickSelection = true;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "TwinPilot|Interaction")
	TObjectPtr<AActor> SelectedActor;

	UPROPERTY(BlueprintReadOnly, Category = "TwinPilot|Interaction")
	TObjectPtr<AActor> HoveredActor;

private:
	void ApplyMouseCursorPolicy();

	bool bRotateHeld = false;
};
