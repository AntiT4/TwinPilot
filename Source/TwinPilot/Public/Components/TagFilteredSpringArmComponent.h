#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "TagFilteredSpringArmComponent.generated.h"

UCLASS(ClassGroup = Camera, meta = (BlueprintSpawnableComponent))
class TWINPILOT_API UTagFilteredSpringArmComponent : public USpringArmComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraCollision")
	FName CollisionTargetActorTag = TEXT("Background");

protected:
	virtual void UpdateDesiredArmLocation(
		bool bDoTrace,
		bool bDoLocationLag,
		bool bDoRotationLag,
		float DeltaTime) override;

private:
	bool SweepForTaggedBlockingHit(const FVector& Start, const FVector& End, FHitResult& OutHit) const;
	bool ShouldUseBlockingHit(const FHitResult& Hit) const;
};
