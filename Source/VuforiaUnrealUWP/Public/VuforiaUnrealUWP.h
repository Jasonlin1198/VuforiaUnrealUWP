// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

//#include "CoreMinimal.h"
#include "Core.h"

#include "Modules/ModuleManager.h"

class FVuforiaUnrealUWPModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	bool SearchForDllPath(FString _searchBase, FString _dllName);

};
