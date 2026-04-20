#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DigitalTwinOperatorController.generated.h"

class AActor;

USTRUCT(BlueprintType)
struct TWINPILOT_API FTwinPilotInteractionState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "TwinPilot|Interaction")
	TObjectPtr<AActor> SelectedActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "TwinPilot|Interaction")
	TObjectPtr<AActor> SelectedHighlightedActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "TwinPilot|Interaction")
	TObjectPtr<AActor> ConfirmedActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "TwinPilot|Interaction")
	TObjectPtr<AActor> ConfirmedHighlightedActor = nullptr;
};

USTRUCT(BlueprintType)
struct TWINPILOT_API FTwinPilotInteractionStateChange
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "TwinPilot|Interaction")
	FTwinPilotInteractionState PreviousState;

	UPROPERTY(BlueprintReadOnly, Category = "TwinPilot|Interaction")
	FTwinPilotInteractionState CurrentState;

	UPROPERTY(BlueprintReadOnly, Category = "TwinPilot|Interaction")
	TObjectPtr<AActor> NewlyConfirmedActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "TwinPilot|Interaction")
	TObjectPtr<AActor> ReleasedConfirmedActor = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTwinPilotActorSelectedSignature, AActor*, SelectedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTwinPilotActorHoveredSignature, AActor*, HoveredActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FTwinPilotInteractionStateChangedSignature,
	const FTwinPilotInteractionStateChange&,
	InteractionStateChange);

UCLASS(Blueprintable)
class TWINPILOT_API ADigitalTwinOperatorController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Interaction")
	bool ConfirmActorUnderCursor(AActor*& HitActor);

	UFUNCTION(
		BlueprintCallable,
		Category = "TwinPilot|Interaction",
		meta = (DeprecatedFunction, DeprecationMessage = "Use ConfirmActorUnderCursor instead."))
	UE_DEPRECATED(5.7, "Use ConfirmActorUnderCursor instead.")
	bool SelectActorUnderCursor(AActor*& HitActor);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Interaction")
	bool SelectUnderCursor();

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Interaction")
	bool ClearSelectedActor();

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Interaction")
	bool ClearConfirmedActor();

	UFUNCTION(
		BlueprintCallable,
		Category = "TwinPilot|Interaction",
		meta = (DeprecatedFunction, DeprecationMessage = "Use SelectUnderCursor instead."))
	UE_DEPRECATED(5.7, "Use SelectUnderCursor instead.")
	bool PreselectActorUnderCursor();

	UFUNCTION(
		BlueprintCallable,
		Category = "TwinPilot|Interaction",
		meta = (DeprecatedFunction, DeprecationMessage = "Use SelectUnderCursor instead."))
	UE_DEPRECATED(5.7, "Use SelectUnderCursor instead.")
	bool HoverActorUnderCursor();

	UFUNCTION(BlueprintPure, Category = "TwinPilot|Interaction")
	AActor* GetSelectedActor() const { return SelectedActor; }

	UFUNCTION(BlueprintPure, Category = "TwinPilot|Interaction")
	AActor* GetSelectedHighlightedActor() const { return SelectedHighlightedActor; }

	UFUNCTION(BlueprintPure, Category = "TwinPilot|Interaction")
	AActor* GetConfirmedActor() const { return ConfirmedActor; }

	UFUNCTION(BlueprintPure, Category = "TwinPilot|Interaction")
	AActor* GetConfirmedHighlightedActor() const { return ConfirmedHighlightedActor; }

	UFUNCTION(BlueprintPure, Category = "TwinPilot|Interaction")
	FTwinPilotInteractionState GetInteractionState() const;

	UFUNCTION(BlueprintPure, Category = "TwinPilot|Interaction")
	FTwinPilotInteractionStateChange GetLastInteractionStateChange() const { return LastInteractionStateChange; }

	UFUNCTION(
		BlueprintPure,
		Category = "TwinPilot|Interaction",
		meta = (DeprecatedFunction, DeprecationMessage = "Use GetSelectedActor instead."))
	UE_DEPRECATED(5.7, "Use GetSelectedActor instead.")
	AActor* GetHoveredActor() const { return SelectedActor; }

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Camera")
	void SetRotateHeld(bool bHeld);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Camera")
	void SetPanHeld(bool bHeld);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Camera")
	void AddPanInputDelta(float ScreenXDelta, float ScreenYDelta);

	UPROPERTY(BlueprintAssignable, Category = "TwinPilot|Interaction")
	FTwinPilotActorSelectedSignature OnActorSelected;

	UPROPERTY(BlueprintAssignable, Category = "TwinPilot|Interaction")
	FTwinPilotActorHoveredSignature OnActorHovered;

	UPROPERTY(BlueprintAssignable, Category = "TwinPilot|Interaction")
	FTwinPilotInteractionStateChangedSignature OnInteractionStateChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinPilot|Interaction")
	FName SelectionBlockedTag = TEXT("Background");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinPilot|Mouse")
	bool bAlwaysShowCursor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TwinPilot|Mouse")
	bool bEnableMouseClickSelection = true;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "TwinPilot|Interaction")
	TObjectPtr<AActor> ConfirmedActor;

	UPROPERTY(BlueprintReadOnly, Category = "TwinPilot|Interaction")
	TObjectPtr<AActor> ConfirmedHighlightedActor;

	UPROPERTY(BlueprintReadOnly, Category = "TwinPilot|Interaction")
	TObjectPtr<AActor> SelectedActor;

	UPROPERTY(BlueprintReadOnly, Category = "TwinPilot|Interaction")
	TObjectPtr<AActor> SelectedHighlightedActor;

	UPROPERTY(BlueprintReadOnly, Category = "TwinPilot|Interaction")
	FTwinPilotInteractionStateChange LastInteractionStateChange;

private:
	void ApplyMouseCursorPolicy();
	bool GetSelectableActorUnderCursor(AActor*& HitActor) const;
	bool ShouldIgnoreActorForSelection(const AActor* Actor) const;
	void UpdateHighlightedActors();
	void RefreshInteractionHighlights(const FTwinPilotInteractionState& PreviousState);
	void NotifyInteractionStateChange(const FTwinPilotInteractionState& PreviousState);

	bool bRotateHeld = false;
};
