#include "Libraries/DigitalTwinPawnLibrary.h"

#include "Pawn/DigitalTwinOperatorPawn.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

void UDigitalTwinPawnLibrary::SetOperatorPawnMoveSpeed(ADigitalTwinOperatorPawn* Pawn, float NewMoveSpeed)
{
	if (Pawn == nullptr)
	{
		return;
	}

	Pawn->SetMoveSpeed(NewMoveSpeed);
}

bool UDigitalTwinPawnLibrary::FocusOperatorPawnOnActor(ADigitalTwinOperatorPawn* Pawn, const AActor* TargetActor, float DistanceMultiplier)
{
	if (Pawn == nullptr || TargetActor == nullptr)
	{
		return false;
	}

	float SphereRadius = 0.0f;
	const FVector Center = GetActorBoundsCenter(TargetActor, SphereRadius);
	const float SafeMultiplier = FMath::Max(1.0f, DistanceMultiplier);
	const float DesiredDistance = FMath::Max(150.0f, SphereRadius * SafeMultiplier);

	Pawn->FocusAtLocation(Center, DesiredDistance);
	return true;
}

bool UDigitalTwinPawnLibrary::TeleportPawnNearActor(APawn* Pawn, const AActor* TargetActor, FVector LocalOffset)
{
	if (Pawn == nullptr || TargetActor == nullptr)
	{
		return false;
	}

	const FVector WorldOffset = TargetActor->GetActorTransform().TransformVectorNoScale(LocalOffset);
	const FVector Destination = TargetActor->GetActorLocation() + WorldOffset;
	return Pawn->SetActorLocation(Destination, false, nullptr, ETeleportType::TeleportPhysics);
}

void UDigitalTwinPawnLibrary::SetPlayerLookInputEnabled(APlayerController* PlayerController, bool bEnabled)
{
	if (PlayerController == nullptr)
	{
		return;
	}

	PlayerController->SetIgnoreLookInput(!bEnabled);
}

FVector UDigitalTwinPawnLibrary::GetActorBoundsCenter(const AActor* TargetActor, float& OutSphereRadius)
{
	OutSphereRadius = 0.0f;

	if (TargetActor == nullptr)
	{
		return FVector::ZeroVector;
	}

	FVector Origin = FVector::ZeroVector;
	FVector BoxExtent = FVector::ZeroVector;
	TargetActor->GetActorBounds(true, Origin, BoxExtent);
	OutSphereRadius = BoxExtent.Size();
	return Origin;
}
