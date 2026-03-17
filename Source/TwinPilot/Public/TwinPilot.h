#pragma once

#include "Modules/ModuleManager.h"

class FTwinPilotModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
