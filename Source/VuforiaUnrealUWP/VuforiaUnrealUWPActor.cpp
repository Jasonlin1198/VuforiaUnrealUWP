// Project
#include "VuforiaUnrealUWPActor.h"
#include "VuforiaUnrealUWPUtils.h"

// Unreal Engine 
#include "Camera/CameraComponent.h"
#include "Camera/CameraTypes.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/DefaultPawn.h"
#include "SceneView.h"
#include "Misc/Paths.h"

THIRD_PARTY_INCLUDES_START
// Initialize
#include <Vuforia/Vuforia.h>

// UWP
#include <Vuforia/UWP/Vuforia_UWP.h>
#include <Vuforia/UWP/DXRenderer.h>

// Trackers
#include <Vuforia/Tracker.h>
#include <Vuforia/Trackable.h>
#include <Vuforia/TrackableResult.h>
#include <Vuforia/TrackerManager.h>
#include <Vuforia/PositionalDeviceTracker.h>
#include <Vuforia/ObjectTracker.h>
#include <Vuforia/DeviceTrackableResult.h>

// Target
#include <Vuforia/ModelTarget.h>
#include <Vuforia/ImageTargetResult.h>
#include <Vuforia/ModelTargetResult.h>

// State
#include <Vuforia/State.h>
#include <Vuforia/StateUpdater.h>

// Device
#include <Vuforia/Device.h>
#include <Vuforia/EyewearDevice.h>

#include <Vuforia/DataSet.h>
#include <Vuforia/CameraDevice.h>

#include <Vuforia/Tool.h>
#include <Vuforia/Image.h>
#include <Vuforia/Frame.h>
#include <Vuforia/Renderer.h>
#include <Vuforia/VideoBackgroundTextureInfo.h>
#include <Vuforia/VideoBackgroundConfig.h>
#include <Vuforia/UpdateCallback.h>
#include <Vuforia/Matrices.h>
#include <Vuforia/RenderingPrimitives.h>
#include <Vuforia/GuideView.h>
#include <Vuforia/Image.h>
THIRD_PARTY_INCLUDES_END

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

using namespace winrt::Windows::Perception;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::Graphics::Holographic;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <string>
#include <windows.h>


namespace {
	constexpr char licenseKey[] = "AVUUtdr/////AAABmVYAZEopnUpJs5SuKrDRvAIvazpGTLd95mB3qRdVTkIUcr9UUu1W9oUBgylfrN4Lu5jFJmuqzxy+m4s6Cs0jZm+0Mh6CqzsVzzjXtyV3iqUjntm0sdjcNKnUnHg0CC+1thEi/YbRrsIcM9n3dHcbGuK/Hy2m1iHKzVFEAxRI3Dj1IUK+c3kMbN3TvJznZw3znw+/MKQq3mNI7jBdOZlBt0TIurwuxQQbLAc8PMz/0lWtTa7vGkY6wt/7k5CE/DYWoccLCP5ULHc6D0vXnzyVfwxiP9l5O5RmiCC5EHwCz8oaWvwmehyOOjplSpIAGh7uJTUFAfg9cm1+PGPwcGl/QIoB3j+SuzmpavvkSlHYwx7M";
}

///////////////////////////////////////// UNREAL ENGINE ACTOR FUNCTIONS ////////////////////////////////////////////

AVuforiaActor::AVuforiaActor()
{
	// Set this actor to call Tick() every frame. 
	PrimaryActorTick.bCanEverTick = true;

}


void AVuforiaActor::BeginPlay()
{
	// Call CameraActor's BeginPlay(), gets index of the player for whom we auto-activate and binds to camera. This is set in the blueprint details under Auto Player Activation
	Super::BeginPlay();

	UE_LOG(VuforiaLog, Warning, TEXT("AVuforiaActor::BeginPlay"));


	UE_LOG(VuforiaLog, Warning, TEXT("AVuforiaActor::Finished Begin Play"));
}


// Called every frame
void AVuforiaActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


///////////////////////////////////////// VUFORIA FUNCTIONS ////////////////////////////////////////////


/*
	Perform all initialization steps for Vuforia
*/
bool AVuforiaActor::initAR()
{
	if (!initVuforiaInternal())
	{
		GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Blue, TEXT("Failed: initVuforiaInternal"));

		return false;
	}
	if (!initTrackers())
	{
		GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Blue, TEXT("Failed: initTrackers"));

		return false;
	}

	if (!loadTrackerData())
	{
		GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Red, TEXT("Failed: loadTrackerData"));

		return false;
	}

	GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Green, TEXT("SUCCESSFUL INITIALIZATION"));

	return true;
}

/* 
	Initialize Vuforia with License Key Provided in Settings 
*/
bool AVuforiaActor::initVuforiaInternal() {

	Vuforia::setInitParameters(licenseKey);

	// Vuforia::init() will return positive numbers up to 100 as it progresses towards success.
	// Negative numbers indicate error conditions
	int progress = 0;
	while (progress >= 0 && progress < 100)
	{
		progress = Vuforia::init();
		GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Yellow, TEXT("Initializing"));
	}

	if (progress == 100)
	{
		// Confirm that Vuforia is initialized
		if (Vuforia::isInitialized()) {
			UE_LOG(VuforiaLog, Warning, TEXT("AVuforiaActor::Is Initialized"));
			GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Green, TEXT("Confirmed that Vuforia is Initialized")); \

			// Gets the SpatialLocator instance that tracks the location of the current device, such as a HoloLens, relative to the user's surroundings.
			SpatialLocator locator = SpatialLocator::GetDefault();

			// The simplest way to render world-locked holograms is to create a stationary reference frame
			// when the app is launched. This is roughly analogous to creating a "world" coordinate system
			// with the origin placed at the device's position as the app is launched.

			// Creates a frame of reference that remains stationary relative to the user's surroundings, with its initial origin at the SpatialLocator's current location.
			SpatialStationaryFrameOfReference ssfr = locator.CreateStationaryFrameOfReferenceAtCurrentLocation();

			SpatialCoordinateSystem cs = ssfr.CoordinateSystem();
			if (cs == nullptr)
			{
				GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Blue, TEXT("COORDINATE SYSTEM NULL"));
			}

			if (!Vuforia::setHolographicAppCS(&cs))
			{
				GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Blue, TEXT("SET HOLOGRAPHIC CS FAILED"));

			}
			else {
				GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Green, TEXT("SET HOLOGRAPHIC CS SUCCESS"));
			}

			return true;
		}
		else {
			UE_LOG(VuforiaLog, Warning, TEXT("AVuforiaActor::Failed Initialized"));
			GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Red, TEXT("Failed Vuforia Initialization"));

			return false;
		}
	}

	// Failed to initialize Vuforia Engine and returned negative number in progress:
	std::string cameraAccessErrorMessage = "";

	switch (progress)
	{
	case Vuforia::INIT_NO_CAMERA_ACCESS:
		// On most platforms the user must explicitly grant camera access
		// If the access request is denied this code is returned
		cameraAccessErrorMessage = "Vuforia cannot initialize because access to the camera was denied.";
		break;

	case Vuforia::INIT_LICENSE_ERROR_NO_NETWORK_TRANSIENT:
		cameraAccessErrorMessage = "Vuforia failed to initialize because the license check encountered a temporary network error.";
		break;

	case Vuforia::INIT_LICENSE_ERROR_NO_NETWORK_PERMANENT:
		cameraAccessErrorMessage = "Vuforia failed to initialize because the license check encountered a permanent network error.";
		break;

	case Vuforia::INIT_LICENSE_ERROR_INVALID_KEY:
		cameraAccessErrorMessage = "Vuforia failed to initialize because the license key is invalid.";
		break;

	case Vuforia::INIT_LICENSE_ERROR_CANCELED_KEY:
		cameraAccessErrorMessage = "Vuforia failed to initialize because the license key was cancelled.";
		break;

	case Vuforia::INIT_LICENSE_ERROR_MISSING_KEY:
		cameraAccessErrorMessage = "Vuforia failed to initialize because the license key was missing.";
		break;

	case Vuforia::INIT_LICENSE_ERROR_PRODUCT_TYPE_MISMATCH:
		cameraAccessErrorMessage = "Vuforia failed to initialize because the license key is for the wrong product type.";
		break;

	case Vuforia::INIT_DEVICE_NOT_SUPPORTED:
		cameraAccessErrorMessage = "Vuforia failed to initialize because the device is not supported.";
		break;

	default:
		cameraAccessErrorMessage = "Vuforia initialization failed.";
		break;
	}
	
	UE_LOG(VuforiaLog, Warning, TEXT("%s"), cameraAccessErrorMessage.c_str());

	return false;
	
}


/*
	Setup Vuforia Tracker Manager and Device/Object Trackers.
	A tracker is a core computing unit in the Vuforia API which tracks and reports the poses of objects and devices in the user's environment.

	PositionalDeviceTracker - type of DeviceTracker that tracks position and orientation of head-mounted displays
	ObjectTracker - tracks ObjectTargets which include imageTargets, modelTargets, and VuMarkers.

*/
bool AVuforiaActor::initTrackers() {

	// TrackerManager is available after Vuforia::init() has succeeded
	Vuforia::TrackerManager& trackerManager = Vuforia::TrackerManager::getInstance();
	Vuforia::Tracker* positionalTracker = trackerManager.initTracker(Vuforia::PositionalDeviceTracker::getClassType());

	// Check Initialization of Device Tracker
	if (positionalTracker == nullptr)
	{
		return false;
	}

	// Check Initialization of Object Tracker
	Vuforia::Tracker* objectTracker = trackerManager.initTracker(Vuforia::ObjectTracker::getClassType());
	if (objectTracker == NULL)
	{
		return false;
	}

	return true;
}


/*
	Load DataSet if one has not yet been loaded
*/
bool AVuforiaActor::loadTrackerData()
{
	if (currentDataSet != nullptr)
	{
		// Attempt to load a dataset when one is already loaded
		return false;
	}

	currentDataSet = loadAndActivateDataSet("C:/data/Programs/WindowsApps/ARNOC_1.0.0.0_ARM64__saeq9htvs62ct/HololensApp/Content/Datasets/StonesAndChips.xml");
	if (currentDataSet == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Orange, TEXT("Failed to load dataset"));

		// Error loading dataset for Image Target
		return false;
	}

	return true;
}

/*
	Load DataSet from xml file path from resources and Activate
*/
Vuforia::DataSet* AVuforiaActor::loadAndActivateDataSet(std::string path)
{
	// Loading dataset from path
	Vuforia::DataSet* dataSet = nullptr;

	// Get the Vuforia tracker manager image tracker
	Vuforia::TrackerManager& trackerManager = Vuforia::TrackerManager::getInstance();
	Vuforia::ObjectTracker* objectTracker = static_cast<Vuforia::ObjectTracker*>(trackerManager.getTracker(Vuforia::ObjectTracker::getClassType()));

	if (objectTracker == nullptr)
	{
		// Failed to get the ObjectTracker from the TrackerManager
	}
	else
	{
		dataSet = objectTracker->createDataSet();
		if (dataSet == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Yellow, TEXT("Failed to create dataset"));
		}
		else
		{
			// Check if dataset exists at absolute file location. Datasets are defined as .xml and .dat files in the same folder and are searched by the .xml file path
			if (dataSet->exists(path.c_str(), Vuforia::STORAGE_ABSOLUTE)) {
				// Load the dataset from the packaged resources
				if (dataSet->load(path.c_str(), Vuforia::STORAGE_ABSOLUTE))
				{
					if (!objectTracker->activateDataSet(dataSet))
					{
						// Failed to activate data set
						objectTracker->destroyDataSet(dataSet);
						dataSet = nullptr;
						GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Red, TEXT("Failed to activate dataset"));

					}
				}
				else
				{
					// Failed to load data set
					objectTracker->destroyDataSet(dataSet);
					dataSet = nullptr;
					GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Orange, TEXT("Failed to load dataset"));
				}
			}
			else {
				objectTracker->destroyDataSet(dataSet);
				dataSet = nullptr;
				GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Purple, TEXT("DataSet does not exist"));
			}
		}
	}

	return dataSet;
}


/*
	Initialize Camera, Start Trackers, and Start Camera
*/
bool AVuforiaActor::startAR()
{

	if (isCameraStarted || isCameraActive)
	{
		GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Blue, TEXT("----------------------------CAMERA ALREADY ACTIVE"));

		// Application logic error, attempt to startAR when already started
		return false;
	}
	Vuforia::CameraDevice& camera = Vuforia::CameraDevice::getInstance();
	if (&camera == nullptr) {
		GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Blue, TEXT("----------------------------CAMERA INSTANCE"));
	}

	// initialize the camera
	if (!Vuforia::CameraDevice::getInstance().init())
	{
		GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Blue, TEXT("----------------------------CAMERA INIT"));

		// Failed to initialize the camera
		return false;
	}

	// select the default video mode
	if (!Vuforia::CameraDevice::getInstance().selectVideoMode(cameraMode))
	{
		GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Blue, TEXT("----------------------------CAMERA MODE"));

		// Failed to set the camera mode
		return false;
	}

	//// set the FPS to its recommended value
	//int recommendedFps = Vuforia::Renderer::getInstance().getRecommendedFps();
	//Vuforia::Renderer::getInstance().setTargetFps(recommendedFps);

	if (!startTrackers())
	{
		GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Blue, TEXT("----------------------------TRACKERS"));

		// Failed to start trackers
		return false;
	}

	if (!Vuforia::CameraDevice::getInstance().start())
	{
		GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Blue, TEXT("----------------------------CAMERA START"));

		// Failed to start the camera 
		return false;
	}

	// Set camera to autofocus
	if (!Vuforia::CameraDevice::getInstance().setFocusMode(Vuforia::CameraDevice::FOCUS_MODE_CONTINUOUSAUTO))
	{
		// Failed to set camera to continuous autofocus, camera may not support this
	}

	GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Blue, TEXT("AR STARTED"));

	isCameraActive = true;
	isCameraStarted = true;
	return true;

}

/*
	Start PositionalDeviceTracker and ObjectTracker
*/
bool AVuforiaActor::startTrackers()
{
	Vuforia::TrackerManager& trackerManager = Vuforia::TrackerManager::getInstance();
	Vuforia::Tracker* deviceTracker = trackerManager.getTracker(Vuforia::PositionalDeviceTracker::getClassType());
	if (deviceTracker != 0)
	{
		deviceTracker->start();
	}
	Vuforia::Tracker* tracker = trackerManager.getTracker(Vuforia::ObjectTracker::getClassType());
	if (tracker == 0)
	{
		return false;
	}

	tracker->start();
	return true;
}


///////////////////////////////////////// UNREAL ENGINE BLUEPRINT FUNCTIONS ////////////////////////////////////////////


// Unreal blueprint callable function for custom vuforia initialization
void AVuforiaActor::BeginInitVuforia()
{
	initAR();
}


// Unreal blueprint callable function for custom vuforia camera initialization
void AVuforiaActor::BeginCameraVuforia()
{
	if (isCameraStarted || isCameraActive)
	{
		GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Blue, TEXT("----------------------------CAMERA ALREADY ACTIVE"));
	}

	Vuforia::CameraDevice& camera = Vuforia::CameraDevice::getInstance();
	if (&camera == nullptr) {
		GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Blue, TEXT("----------------------------CAMERA INSTANCE"));
	}

	if (!Vuforia::CameraDevice::getInstance().init())
	{
		GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Blue, TEXT("----------------------------CAMERA INIT"));
	}

	if (!Vuforia::CameraDevice::getInstance().selectVideoMode(cameraMode))
	{
		GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Blue, TEXT("----------------------------CAMERA MODE"));
	}

	if (!startTrackers())
	{
		GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Blue, TEXT("----------------------------TRACKERS"));
	}

	if (!Vuforia::CameraDevice::getInstance().start())
	{
		GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Blue, TEXT("----------------------------CAMERA START"));
	}

	if (!Vuforia::CameraDevice::getInstance().setFocusMode(Vuforia::CameraDevice::FOCUS_MODE_CONTINUOUSAUTO))
	{
		// Failed to set camera to continuous autofocus, camera may not support this
	}
}

///////////////////////////////////////// Vuforia4Unreal FUNCTIONS ////////////////////////////////////////////

void AVuforiaActor::HideDefaultPawnCollisionComponent()
{
	UE_LOG(VuforiaLog, Warning, TEXT("AVuforiaActor::HidingDefaultPawnCollisionComponent"))

		// Gets all default pawn actors in the world and makes the sphere mesh component non-visible 
		for (TActorIterator<ADefaultPawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			ADefaultPawn* pawn = *ActorItr;

			UE_LOG(VuforiaLog, Warning, TEXT("AVuforiaActor::Pawn Initial Location: %s"), *pawn->GetActorLocation().ToString())

				GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Purple, TEXT("Hiding Default Pawn"));

			pawn->GetMeshComponent()->SetVisibility(false);
		}
}