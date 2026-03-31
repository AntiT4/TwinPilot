#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TwinPilotPawnControl.generated.h"

class AActor;

UINTERFACE(Blueprintable)
class TWINPILOT_API UTwinPilotPawnControl : public UInterface
{
	GENERATED_BODY()
};

class TWINPILOT_API ITwinPilotPawnControl
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "TwinPilot|Control")
	void HandleConfirmedActor(const AActor* TargetActor);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "TwinPilot|Control")
	void SetOrbitHeld(bool bHeld);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "TwinPilot|Control")
	void SetPanHeld(bool bHeld);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "TwinPilot|Control")
	void AddPanInputDelta(float ScreenXDelta, float ScreenYDelta);
};
