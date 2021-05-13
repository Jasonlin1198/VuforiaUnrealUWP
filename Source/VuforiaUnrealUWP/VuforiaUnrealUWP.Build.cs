// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class VuforiaUnrealUWP : ModuleRules
{
	public VuforiaUnrealUWP(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);

		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
				PrivateIncludePath,
			}
			) ;
			
		
		// Additional Modules described by .Build.cs files to include in plugin. Ex. Third Party Libraries
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore", 
				"RHI", 
				"RenderCore",
				"Projects",
				"VuforiaSDK"

				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"ProceduralMeshComponent"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
	private string PrivateIncludePath
	{
		get { return Path.GetFullPath(Path.Combine(ModulePath, "Private")); }
	}
	private string ModulePath
	{
		get	{ return ModuleDirectory; }
	}
}
