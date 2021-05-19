// Copyright Epic Games, Inc. All Rights Reserved.

#include "VuforiaUnrealUWP.h"
#include "VuforiaUnrealUWPSettings.h"
#include "VuforiaUnrealUWPUtils.h"

#include "Developer/Settings/Public/ISettingsModule.h"
#include "Developer/Settings/Public/ISettingsSection.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FVuforiaUnrealUWPModule"

void FVuforiaUnrealUWPModule::StartupModule()
{
	//Search project plugins folder for Dll
	FString dllName = "Vuforia.dll";
	if (SearchForDllPath(FPaths::ProjectPluginsDir(), dllName))
	{
		UE_LOG(VuforiaLog, Warning, TEXT("StartupModule::Successfully DLL Search"));
	}
	//Failed in project dir, try engine plugins dir
	else if (SearchForDllPath(FPaths::EnginePluginsDir(), dllName)) 
	{
		UE_LOG(VuforiaLog, Warning, TEXT("StartupModule::Successfully DLL Search - Found Engine"));
	}
	else
	{
		//Stop loading - plugin required DLL to load successfully
		UE_LOG(VuforiaLog, Warning, TEXT("StartupModule::Failed DLL Search"));
	}

	// register settings
	auto settingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (settingsModule)
	{
		settingsModule->RegisterSettings("Project", "Plugins", "Vuforia",
			LOCTEXT("VuforiaSettingsName", "Vuforia"),
			LOCTEXT("VuforiaSettingsDescription", "Configure the Vuforia plug-in."),
			GetMutableDefault<UVuforiaSettings>());
	}

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FVuforiaUnrealUWPModule::ShutdownModule()
{

	// unregister settings
	auto settingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (settingsModule)
	{
		settingsModule->UnregisterSettings("Project", "Plugins", "Vuforia");
	}

	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVuforiaUnrealUWPModule, VuforiaUnrealUWP)


bool FVuforiaUnrealUWPModule::SearchForDllPath(FString _searchBase, FString _dllName)
{
	//Search Plugins folder for an instance of Dll.dll, and add to platform search path
	TArray<FString> directoriesToSkip;
	IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FLocalTimestampDirectoryVisitor Visitor(PlatformFile, directoriesToSkip, directoriesToSkip, false);
	PlatformFile.IterateDirectory(*_searchBase, Visitor);

	for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
	{
		const FString file = TimestampIt.Key();
		const FString filePath = FPaths::GetPath(file);
		const FString fileName = FPaths::GetCleanFilename(file);
		if (fileName.Compare(_dllName) == 0)
		{
			FPlatformProcess::AddDllDirectory(*filePath); // only load dll when needed for use. Broken with 4.11.
			FPlatformProcess::GetDllHandle(*file); // auto-load dll with plugin - needed as 4.11 breaks above line.
			UE_LOG(VuforiaLog, Warning, TEXT("Added DLL Handle"));

			return true;
		}
	}
	return false;
}
