#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DigitalTwinPawnLibrary.generated.h"

class AActor;
class ADigitalTwinOperatorPawn;
class APawn;
class APlayerController;

UCLASS()
class TWINPILOT_API UDigitalTwinPawnLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Pawn")
	static void SetOperatorPawnMoveSpeed(ADigitalTwinOperatorPawn* Pawn, float NewMoveSpeed);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Pawn")
	static bool FocusOperatorPawnOnActor(ADigitalTwinOperatorPawn* Pawn, const AActor* TargetActor, float DistanceMultiplier = 2.5f);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Pawn")
	static bool TeleportPawnNearActor(APawn* Pawn, const AActor* TargetActor, FVector LocalOffset);

	UFUNCTION(BlueprintCallable, Category = "TwinPilot|Control")
	static void SetPlayerLookInputEnabled(APlayerController* PlayerController, bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "TwinPilot|Interaction", meta = (DefaultToSelf = "PlayerController"))
	static bool GetActorUnderCursor(
		APlayerController* PlayerController,
		AActor*& OutActor,
		ECollisionChannel TraceChannel = ECC_Visibility,
		bool bTraceComplex = true);

	UFUNCTION(BlueprintPure, Category = "TwinPilot|Math")
	static FVector GetActorBoundsCenter(const AActor* TargetActor, float& OutSphereRadius);
};
