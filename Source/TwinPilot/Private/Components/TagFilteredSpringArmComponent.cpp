#include "Components/TagFilteredSpringArmComponent.h"

#include "DrawDebugHelpers.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Math/RotationMatrix.h"
#include "PhysicsEngine/PhysicsSettings.h"

void UTagFilteredSpringArmComponent::UpdateDesiredArmLocation(
	bool bDoTrace,
	bool bDoLocationLag,
	bool bDoRotationLag,
	float DeltaTime)
{
	FRotator DesiredRot = GetTargetRotation();

	if (bClampToMaxPhysicsDeltaTime)
	{
		DeltaTime = FMath::Min(DeltaTime, UPhysicsSettings::Get()->MaxPhysicsDeltaTime);
	}

	if (bDoRotationLag)
	{
		if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraRotationLagSpeed > 0.0f)
		{
			const FRotator ArmRotStep = (DesiredRot - PreviousDesiredRot).GetNormalized() * (1.0f / DeltaTime);
			FRotator LerpTarget = PreviousDesiredRot;
			float RemainingTime = DeltaTime;
			while (RemainingTime > UE_KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
				LerpTarget += ArmRotStep * LerpAmount;
				RemainingTime -= LerpAmount;

				DesiredRot = FRotator(FMath::QInterpTo(
					FQuat(PreviousDesiredRot),
					FQuat(LerpTarget),
					LerpAmount,
					CameraRotationLagSpeed));
				PreviousDesiredRot = DesiredRot;
			}
		}
		else
		{
			DesiredRot = FRotator(FMath::QInterpTo(
				FQuat(PreviousDesiredRot),
				FQuat(DesiredRot),
				DeltaTime,
				CameraRotationLagSpeed));
		}
	}
	PreviousDesiredRot = DesiredRot;

	FVector ArmOrigin = GetComponentLocation() + TargetOffset;
	FVector DesiredLoc = ArmOrigin;
	if (bDoLocationLag)
	{
		if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraLagSpeed > 0.0f)
		{
			const FVector ArmMovementStep = (DesiredLoc - PreviousDesiredLoc) * (1.0f / DeltaTime);
			FVector LerpTarget = PreviousDesiredLoc;

			float RemainingTime = DeltaTime;
			while (RemainingTime > UE_KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
				LerpTarget += ArmMovementStep * LerpAmount;
				RemainingTime -= LerpAmount;

				DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, LerpTarget, LerpAmount, CameraLagSpeed);
				PreviousDesiredLoc = DesiredLoc;
			}
		}
		else
		{
			DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, DesiredLoc, DeltaTime, CameraLagSpeed);
		}

		bool bClampedDist = false;
		if (CameraLagMaxDistance > 0.0f)
		{
			const FVector FromOrigin = DesiredLoc - ArmOrigin;
			if (FromOrigin.SizeSquared() > FMath::Square(CameraLagMaxDistance))
			{
				DesiredLoc = ArmOrigin + FromOrigin.GetClampedToMaxSize(CameraLagMaxDistance);
				bClampedDist = true;
			}
		}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (bDrawDebugLagMarkers)
		{
			DrawDebugSphere(GetWorld(), ArmOrigin, 5.0f, 8, FColor::Green);
			DrawDebugSphere(GetWorld(), DesiredLoc, 5.0f, 8, FColor::Yellow);

			const FVector ToOrigin = ArmOrigin - DesiredLoc;
			DrawDebugDirectionalArrow(
				GetWorld(),
				DesiredLoc,
				DesiredLoc + ToOrigin * 0.5f,
				7.5f,
				bClampedDist ? FColor::Red : FColor::Green);
			DrawDebugDirectionalArrow(
				GetWorld(),
				DesiredLoc + ToOrigin * 0.5f,
				ArmOrigin,
				7.5f,
				bClampedDist ? FColor::Red : FColor::Green);
		}
#endif
	}

	PreviousArmOrigin = ArmOrigin;
	PreviousDesiredLoc = DesiredLoc;

	DesiredLoc -= DesiredRot.Vector() * TargetArmLength;
	DesiredLoc += FRotationMatrix(DesiredRot).TransformVector(SocketOffset);

	FVector ResultLoc;
	if (bDoTrace && TargetArmLength != 0.0f)
	{
		bIsCameraFixed = true;
		FHitResult Result;
		SweepForTaggedBlockingHit(ArmOrigin, DesiredLoc, Result);

		UnfixedCameraPosition = DesiredLoc;
		ResultLoc = BlendLocations(DesiredLoc, Result.Location, Result.bBlockingHit, DeltaTime);

		if (ResultLoc == DesiredLoc)
		{
			bIsCameraFixed = false;
		}
	}
	else
	{
		ResultLoc = DesiredLoc;
		bIsCameraFixed = false;
		UnfixedCameraPosition = ResultLoc;
	}

	const FTransform WorldCamTM(DesiredRot, ResultLoc);
	const FTransform RelCamTM = WorldCamTM.GetRelativeTransform(GetComponentTransform());

	RelativeSocketLocation = RelCamTM.GetLocation();
	RelativeSocketRotation = RelCamTM.GetRotation();

	UpdateChildTransforms();
}

bool UTagFilteredSpringArmComponent::SweepForTaggedBlockingHit(
	const FVector& Start,
	const FVector& End,
	FHitResult& OutHit) const
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		OutHit = FHitResult();
		return false;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpringArm), false, GetOwner());

	for (int32 Attempts = 0; Attempts < 64; ++Attempts)
	{
		FHitResult Hit;
		World->SweepSingleByChannel(
			Hit,
			Start,
			End,
			FQuat::Identity,
			ProbeChannel,
			FCollisionShape::MakeSphere(ProbeSize),
			QueryParams);

		if (!Hit.bBlockingHit)
		{
			OutHit = Hit;
			return false;
		}

		if (ShouldUseBlockingHit(Hit))
		{
			OutHit = Hit;
			return true;
		}

		if (AActor* IgnoredActor = Hit.GetActor())
		{
			QueryParams.AddIgnoredActor(IgnoredActor);
			continue;
		}

		if (UPrimitiveComponent* IgnoredComponent = Hit.GetComponent())
		{
			QueryParams.AddIgnoredComponent(IgnoredComponent);
			continue;
		}

		break;
	}

	OutHit = FHitResult();
	return false;
}

bool UTagFilteredSpringArmComponent::ShouldUseBlockingHit(const FHitResult& Hit) const
{
	const AActor* HitActor = Hit.GetActor();
	return HitActor != nullptr
		&& !CollisionTargetActorTag.IsNone()
		&& HitActor->ActorHasTag(CollisionTargetActorTag);
}
