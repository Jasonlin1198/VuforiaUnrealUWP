// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class VuforiaSDK : ModuleRules
{
	public VuforiaSDK(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// Tell Unreal that this Module only imports Third-Party-Assets
		Type = ModuleType.External;

		LoadVuforiaSDK(Target);
	}

	public bool LoadVuforiaSDK(ReadOnlyTargetRules Target)
    {
		bool isLibrarySupported = false;
		string platformString = null;

		// Check which platform Unreal is built for
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			platformString = "x64";
		}
		else if (Target.Platform == UnrealTargetPlatform.HoloLens)
		{ 
			platformString = "ARM64";
		}

		// Add dependencies if current platform is supported
		if (platformString != null)
        {
			isLibrarySupported = true;

			// Add the static import library 
			PublicAdditionalLibraries.Add(Path.Combine(ModulePath, "VuforiaSDK", "lib", platformString , "Vuforia.lib"));

			// Add include directory path
			PublicIncludePaths.Add(Path.Combine(ModulePath, "VuforiaSDK", "include"));

			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("Vuforia.dll");

			// Ensure that the DLL is staged along with the executable
			RuntimeDependencies.Add(Path.Combine("$(PluginDir)", "Binaries", "ThirdParty", "Win64", "Vuforia.dll"));
		}
		return isLibrarySupported;

	}

	// ModuleDirectory points to the directory .uplugin is located
	private string ModulePath
	{
		get { return ModuleDirectory; }
	}

}
