#include "GameMode/DigitalTwinOperatorGameModeBase.h"

#include "Controller/DigitalTwinOperatorController.h"
#include "Pawn/DigitalTwinOperatorPawn.h"

ADigitalTwinOperatorGameModeBase::ADigitalTwinOperatorGameModeBase()
{
	DefaultPawnClass = ADigitalTwinOperatorPawn::StaticClass();
	PlayerControllerClass = ADigitalTwinOperatorController::StaticClass();
}
