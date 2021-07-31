# Vuforia for Unreal UWP

Here I document my process of developing an Unreal Engine 4.26.2 Plugin that enables the use of Vuforia into a project targeted for the Hololens 2. 

## Getting Started

First you will need to install the Vuforia Engine for UWP. Here I downloaded the `vuforia-sdk-uwp-9-8-8.zip` from https://developer.vuforia.com/downloads/sdk.

Install Example Third Party Plugin in Unreal Engine by going to: `Edit→Plugins→Create Plugin`


### Resources

Some helpful links to understand UE4 plugins are noted below:

- Vuforia4Unreal using Vuforia Android SDK
https://github.com/codefluegel/vuforia4unreal

- Unreal Engine C++ Tutorial: Plugins
https://www.youtube.com/watch?v=mgFrFdzb7hg

- Unreal Engine 4 Tutorial - How to make a Plugin - Part01 - Setting up the thing (Hot reload support)
https://www.youtube.com/watch?v=jvkFrETmCoA

- Example build.cs file
https://dawnarc.com/2019/01/ue4build.cs-notes/

- PCL Third Party Plugin Notes
http://www.valentinkraft.de/including-the-point-cloud-library-into-unreal-tutorial/

- PCL Third Party Plugin Code
https://github.com/ValentinKraft/Boost_PCL_UnrealThirdPartyPlugin/blob/master/PointCloudRenderer/Source/ThirdParty/PCL.Build.cs



### File Structure for Third Party Plugins in Unreal Engine  

The plugin can be installed into any Unreal Engine project and should follow the same file structure. File hierarchy of the plugin is layed out below:

```css
|-- ProjectDirectory/
    |-- Plugins/
        |-- VuforiaUnrealUWP/
            |-- Resources/
            |-- Source/
                |-- ThirdParty/
                    |-- VuforiaSDK/
                        |-- include/
                            |-- Vuforia/
                                |-- Driver/
                                |-- UWP/
                        |-- lib/
                            |-- ARM64/
                                |-- Vuforia.dll
                                |-- Vuforia.lib
                            |-- x64/
                            |-- x86/
                        |-- VuforiaSDK.Build.cs
                |-- VuforiaUnrealUWP/
                    |-- Private/
                    |-- Public/
                    |-- VuforiaUnrealUWP.Build.cs
                    |-- VuforiaUnrealUWPActor.cpp
                    |-- VuforiaUnrealUWPActor.h
```


**Problem** with delayhtp.cpp throwing an error on play. This is due to the `.dll` file not being loaded on runtime properly. 

To get access to Vuforia functions defined in the `.dll`, add to VuforiaSDK.Build.cs:

```cs
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
			RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/Win64/Vuforia.dll");
		}
		return isLibrarySupported;

	}

	// ModuleDirectory points to the directory .uplugin is located
	private string ModulePath
	{
		get { return ModuleDirectory; }
	}
}
```


The below is needed for loading the `.dll` file at runtime when needed:
```cs
    // Delay-load the DLL, so we can load it from the right place first
    PublicDelayLoadDLLs.Add("Vuforia.dll");

    // Ensure that the DLL is staged along with the executable
    RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/Win64/Vuforia.dll");
```

`/Binaries/ThirdParty/Win64` needs to be manually created and the `lib/x64/Vuforia.dll` needs to be copied into the folder. 




**Problem** - `Import missing: vccorlib140_app.DLL` seen in Output Log after Engine Build.  

1. Copy `vccorlib140.dll` from `C:\Program Files (x86)\Epic Games\Launcher\Engine\Binaries\Win64`.  

2. Rename the copied file to `vccorlib140_app.DLL`. 

3. Copy the file to `C:\Program Files\Epic Games\UE_4.26\Engine\Binaries\Win64`.

4. Rebuild and check if fixed.

### PublicDependencyModuleNames vs PrivateDependencyModuleNames

PublicDependencyModuleNames will be available in the Public and Private folders, but PrivateDependencyModuleNames will only be available in the Private folder.

Link to forum answer [here.](https://answers.unrealengine.com/questions/23384/what-is-the-difference-between-publicdependencymod.html)



## Notes during Development

Set Build Mode to `DevelopmentEditor` in Visual Studio


In attempt to install the vccorlib140.dll file, tried to use older VS version and Visual C++ Redistributables since the .dll is a packages into those installations and is part of the Windows Runtime (WinRT).

Installed the version of Visual Studio Redistributions for 2015, 2017, and 2019 - `vc_redist.x64.exe`:  
https://support.microsoft.com/en-us/topic/the-latest-supported-visual-c-downloads-2647da03-1eea-4433-9aff-95f26a218cc0

Info on redist files c++:  
https://docs.microsoft.com/en-us/cpp/windows/redistributing-visual-cpp-files?view=msvc-160

Downloaded Visual Studio Community 2017 (version 15.9):  
https://my.visualstudio.com/Downloads?q=Visual%20Studio%202017

Deploying Visual C++ Applications:  
https://docs.microsoft.com/en-us/cpp/windows/walkthrough-deploying-a-visual-cpp-application-by-using-a-setup-project?view=msvc-160

Vuforia for Windows 10 Development:  
https://library.vuforia.com/articles/Training/Getting-Started-with-Vuforia-for-Windows-10-Development.html

Crashes due to delayLoad():  
https://answers.unrealengine.com/questions/364156/view.html

Third Party Plugins Unreal Documentation:  
https://docs.unrealengine.com/en-US/ProductionPipelines/BuildTools/UnrealBuildTool/ThirdPartyLibraries/index.html



Build.cs Files are searched by the Unreal Header Tool (UHT) to establish Modules.



Used code for checking .dll locations from:  
https://answers.unrealengine.com/questions/401885/view.html

```cs
void FPluginModule::StartupModule()
 {
     //Search project plugins folder for Dll
     FString dllName = "DllName.dll";
     if (SearchForDllPath(FPaths::GamePluginsDir(), dllName))
     {
     }
     else if (SearchForDllPath(FPaths::EnginePluginsDir(), dllName)) //Failed in project dir, try engine plugins dir
     {
     }
     else
     {
         //Stop loading - plugin required DLL to load successfully
         checkf(false, TEXT("Failed to load dll"));
     }
 }
 
 bool FPluginModule::SearchForDllPath(FString _searchBase, FString _dllName)
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
             return true;
         }
     }
     return false;
 }
 ```




## New Errors


vccorlib140.amd64.pdb not loaded when game plays

vccorlib140.amd64.pdb contains the debug information required to find the source for the module vccorlib140_app.DLL


At Vuforia::init()

Unhandled exception at 0x00007FFB7F7D1208 (ucrtbase.dll) in UE4Editor.exe: An invalid parameter was passed to a function that considers invalid parameters fatal.



NuGet Package:
    Windows.MixedReality.Toolkit.Foundations





## WinRT for Unreal Engine

From `https://docs.microsoft.com/en-us/windows/mixed-reality/develop/unreal/unreal-winrt?tabs=426`:

Add to top plugin module `.Build.cs` file the following:

```c#
if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.HoloLens)
{
	// These parameters are mandatory for winrt support
	bEnableExceptions = true;
	bUseUnity = false;
	CppStandard = CppStandardVersion.Cpp17;
	PublicSystemLibraries.AddRange(new string[] { "shlwapi.lib", "runtimeobject.lib" });
	PrivateIncludePaths.Add(Path.Combine(Target.WindowsPlatform.WindowsSdkDir,        
                                        "Include", 
                                        Target.WindowsPlatform.WindowsSdkVersion, 
                                        "cppwinrt"));
}
```

Add to `.cpp` the winRT headers:

```c++
#if (PLATFORM_WINDOWS || PLATFORM_HOLOLENS) 
//Before writing any code, you need to disable common warnings in WinRT headers
#pragma warning(disable : 5205 4265 4268 4946)

#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/AllowWindowsPlatformAtomics.h"
#include "Windows/PreWindowsApi.h"

#include <unknwn.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Perception.Spatial.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Graphics.Holographic.h>

#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformAtomics.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
```


Install `C++/WinRT Visual Studio Extension` into VS 2019. This provides example project templates and better debugging tools.

Install NuGet Package `Microsoft.Windows.CppWinRT` into VS 2019. 


C++/WinRT allows access to the WinRT api through the `winrt` namepace. 

For example: `winrt::Windows::Perception::Spatial::SpatialLocator::GetDefault();`

Reduce the need to write namespaces by adding them to the top of the file. 

```c++
using namespace winrt::Windows::Perception::Spatial;
```

Now classes and function belonging to those namespaces from the winrt header files can be used as so:

```c++
SpatialLocator::GetDefault();
```

Misc files suggested by intellisense:

```c++
#include "../../../../../../../../../Program Files (x86)/Windows Kits/10/Include/10.0.19041.0/winrt/windows.perception.spatial.h"

#include "../../../../../../../../../Program Files (x86)/Windows Kits/10/Include/10.0.19041.0/cppwinrt/winrt/Windows.Perception.Spatial.h"
```

```c++
	_spatialLocator = ABI::Windows::Perception::Spatial::SpatialLocator::GetDefault();
	_spatialCoordinateSystemPtr = Microsoft.MixedReality.Toolkit.WindowsMixedReality.WindowsMixedRealityUtilities.UtilitiesProvider.ISpatialCoordinateSystemPtr;
	_originFrameOfReference =_spatialLocator->CreateStationaryFrameOfReferenceAtCurrentLocation();

	auto locator = SpatialLocator::GetDefault();

	ABI::Windows::Graphics::Holographic::HolographicDisplay defaultHolographicDisplay;
	= HolographicDisplay::GetDefault();

	//	// SpatialLocator that is attached to the primary camera.
	//ABI::Windows::Perception::Spatial::SpatialLocator* _spatialLocator;

	//// A reference frame attached to the holographic camera.
	//ABI::Windows::Perception::Spatial::SpatialStationaryFrameOfReference* _originFrameOfReference;


	////auto m_stationaryFrameOfReference = locator->CreateStationaryFrameOfReferenceAtCurrentLocation();


	//   // A reference frame attached to the holographic camera.
	//ABI::Windows::Perception::Spatial::SpatialLocatorAttachedFrameOfReference* m_referenceFrame;

	//ABI::Windows::Perception::Spatial::ISpatialCoordinateSystem* a = _spatialLocator->CreateStationaryFrameOfReferenceAtCurrentLocation();
	

```

## Resources

Winrt API (application binary interface):  
https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/interop-winrt-abi

HolographicDisplay Class:  
https://docs.microsoft.com/en-us/uwp/api/windows.graphics.holographic.holographicdisplay?view=winrt-19041

How to create and locate anchors using Azure Spatial Anchors in C++/WinRT:  
https://docs.microsoft.com/en-us/azure/spatial-anchors/how-tos/create-locate-anchors-cpp-winrt


## Extra Code

Print current directory of current file
``` c++
char buf[256];
GetCurrentDirectoryA(256, buf);
FString myString(buf);
GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Blue, myString);

/* Additional paths UE provides */

FPaths::ProjectContentDir());

FPaths::ProjectPluginsDir());

FPaths::ProjectDir());
```

Failed path load
```c++
dataSet->load(TCHAR_TO_ANSI(*FPaths::Combine(FPaths::ProjectDir(), "Datasets", "StonesAndChips.xml")), Vuforia::STORAGE_APPRESOURCE)
```
Path in hololens to application
```
C:\data\Programs\WindowsApps\ARNOC_1.0.0.0_ARM64__saeq9htvs62ct\HololensApp\Binaries\HoloLens

C:\data\Programs\WindowsApps\ARNOC_1.0.0.0_ARM64__saeq9htvs62ct\HololensApp\Content\Datasets
```

GetARPlaneScale() with comments
```c++
FVector2D AVuforiaActor::GetArPlaneScale()
{
	// Get engine viewport
	FViewport* const Viewport = GEngine->GameViewport->ViewportFrame->GetViewport();

	// FSceneViewInitOption struct inherits from FSceneViewProjectionData which has origin vector, view rotation, and projection matrix
	FSceneViewInitOptions ViewInitOptions;
	FMinimalViewInfo camViewInfo;

	// Gets default aspect ratio for active local players created by the engine
	const EAspectRatioAxisConstraint AspectRatioAxisConstraint = GetDefault<ULocalPlayer>()->AspectRatioAxisConstraint;

	GetCameraComponent()->GetCameraView(0, camViewInfo);

	// Set desired camara location and rotation
	camViewInfo.Location = FVector4(10, 0, 0, 0);
	camViewInfo.Rotation = FRotator(0, 0, 0);

	// Get Camera component minimal view information from desired view info, GetCameraView is BlueprintCallable
	GetCameraComponent()->GetCameraView(0, camViewInfo);

	FMinimalViewInfo::CalculateProjectionMatrixGivenView(camViewInfo, AspectRatioAxisConstraint, Viewport, ViewInitOptions);

	FMatrix model(
		FPlane(1.f, 0.f, 0.f, 0.f),
		FPlane(0.f, 1.f, 0.f, 0.f),
		FPlane(0.f, 0.f, 1.f, 0.f),
		FPlane(0.f, 0.f, 0.f, 1.f));

	FMatrix vectorWrap(
		FPlane(1, 0.f, 0.f, 0.f),
		FPlane(1, 0.f, 0.f, 0.f),
		FPlane(0.f, 0.f, 0.f, 0.f),
		FPlane(0.f, 0.f, 0.f, 0.f));

	// Get x and y scale values of Projection Matrix
	FMatrix scale = (ViewInitOptions.ProjectionMatrix * model).Inverse() * vectorWrap;

	// Scales the scale matrix values and returns as 2d vector
	return FVector2D(scale.M[0][0] * 2.f * (float)AR_PLANE_DISTANCE, scale.M[1][0] * 2.f * (float)AR_PLANE_DISTANCE);
}

```

AVuforiaActor() with comments
```c++


// in VuforiaUnrealUWPActor.h file
UVuforiaVideoPlaneComponent* mVideoPlane;

// in VuforiaUnrealUWPActor.cpp file
AVuforiaActor::AVuforiaActor()
{
	// Set this actor to call Tick() every frame. 
	PrimaryActorTick.bCanEverTick = true;

	// Creates VuforiaActorComponet Object named "VuforiaComponent"
	UVuforiaActorComponent* ActorComponent = CreateDefaultSubobject<UVuforiaActorComponent>("VuforiaComponent");

	// Adds component to the Actor blueprint
	AddOwnedComponent(ActorComponent);

	// Uobject is the base class of all UE objects, gets CameraDelegate from VuforiaActorComponent class and binds OnCameraActive function to call when Execute()
	//ActorComponent->CameraDelegate.BindUObject(this, &AVuforiaActor::OnCameraActive);

	// Creates VideoPlaneComponent object named "GeneratedMesh"
	this->mVideoPlane = CreateDefaultSubobject<UVuforiaVideoPlaneComponent>(TEXT("GeneratedMesh"));

	// Assigns mesh to procedural mesh component
	this->mVideoPlane->Create();

	// Attach VideoPlaneComponent to inherited CameraActor Component with attached transform rule
	this->mVideoPlane->AttachToComponent(GetCameraComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	// Add to serialized instance components array so it gets saved
	AddInstanceComponent(mVideoPlane);
}
```

VuforiaUnrealUWP.cpp
```c++

//private:
//	/** Handle to the test dll we will load */
//	void* ExampleLibraryHandle;


//	FString BaseDir = IPluginManager::Get().FindPlugin("VuforiaUnrealUWP")->GetBaseDir();
//
//	// Add on the relative location of the third party dll and load it
//	FString LibraryPath;
//
//
//#if PLATFORM_WINDOWS
//	LibraryPath = FPaths::Combine(*BaseDir, TEXT("/Binaries/ThirdParty/Win64/Vuforia.dll"));
//#endif 
//
//	ExampleLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;
//
//	if (ExampleLibraryHandle)
//	{
//		// Call the test function in the third party library that opens a message box
//		UE_LOG(CFVuforiaLog, Warning, TEXT("StartupModule::Loaded"));
//
//	}
//	else
//	{
//		UE_LOG(CFVuforiaLog, Warning, TEXT("StartupModule::Not loaded"));
//
//	}

```
