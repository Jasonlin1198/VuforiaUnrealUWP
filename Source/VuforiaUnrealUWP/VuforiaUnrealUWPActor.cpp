/*
	Copyright(c) 2016-2017 Codefluegel GMBH All Rights Reserved.
    Distributed under the BSD 3-Clause license.
*/

#include "VuforiaUnrealUWPActor.h"
#include "Private/VuforiaUnrealUWPActorComponent.h"
#include "Private/VuforiaUnrealUWPUtils.h"
#include "Private/VuforiaUnrealUWPVideoPlaneComponent.h"

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

#if PLATFORM_WINDOWS
  #include <Vuforia/Vuforia.h>
  #include <Vuforia/State.h>
  #include <Vuforia/StateUpdater.h>
  #include <Vuforia/Trackable.h>
  #include <Vuforia/TrackableResult.h>
  #include <Vuforia/TrackerManager.h>
  #include <Vuforia/Tool.h>
  #include <Vuforia/CameraDevice.h>
  #include <Vuforia/Image.h>
  #include <Vuforia/Frame.h>
#endif

THIRD_PARTY_INCLUDES_START
#include <ThirdParty/VuforiaSDK/include/Vuforia/Vuforia.h>
#include <ThirdParty/VuforiaSDK/include/Vuforia/UWP/Vuforia_UWP.h>
THIRD_PARTY_INCLUDES_END

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

#include <string>


AVuforiaActor::AVuforiaActor()
{
	// Set this actor to call Tick() every frame. 
	PrimaryActorTick.bCanEverTick = true;

	// Creates VuforiaActorComponet Object named "VuforiaComponent"
	UVuforiaActorComponent* ActorComponent = CreateDefaultSubobject<UVuforiaActorComponent>("VuforiaComponent");

	// Adds component to the Actor blueprint
	AddOwnedComponent(ActorComponent);

	// Uobject is the base class of all UE objects, gets CameraDelegate from VuforiaActorComponent class and binds OnCameraActive function to call when Execute()
	ActorComponent->CameraDelegate.BindUObject(this, &AVuforiaActor::OnCameraActive);

	// Creates VideoPlaneComponent object named "GeneratedMesh"
	this->mVideoPlane = CreateDefaultSubobject<UVuforiaVideoPlaneComponent>(TEXT("GeneratedMesh"));

	// Assigns mesh to procedural mesh component
	this->mVideoPlane->Create();

	// Attach VideoPlaneComponent to inherited CameraActor Component with attached transform rule
	this->mVideoPlane->AttachToComponent(GetCameraComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	// Add to serialized instance components array so it gets saved
	AddInstanceComponent(mVideoPlane);
}

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

void AVuforiaActor::HideDefaultPawnCollisionComponent()
{
	UE_LOG(CFVuforiaLog, Warning, TEXT("AVuforiaActor::HidingDefaultPawnCollisionComponent"))

	// Gets all default pawn actors in the world and makes the sphere mesh component non-visible 
	for (TActorIterator<ADefaultPawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		ADefaultPawn *pawn = *ActorItr;
	
		UE_LOG(CFVuforiaLog, Warning, TEXT("AVuforiaActor::Pawn Initial Location: %s"), *pawn->GetActorLocation().ToString())

		GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Purple, TEXT("Hiding Default Pawn"));

		pawn->GetMeshComponent()->SetVisibility(false);
	}
}


void AVuforiaActor::BeginPlay()
{
	// Call CameraActor's BeginPlay(), gets index of the player for whom we auto-activate and binds to camera. This is set in the blueprint details under Auto Player Activation
	Super::BeginPlay();

	UE_LOG(CFVuforiaLog, Warning, TEXT("AVuforiaActor::BeginPlay"));


	HideDefaultPawnCollisionComponent();

	if (TrackingLost.IsBound())
	{
		TrackingLost.Broadcast();
	}

	UE_LOG(CFVuforiaLog, Warning, TEXT("AVuforiaActor::Finished Begin Play"));

}


bool AVuforiaActor::initVuforiaInternal() {

	Vuforia::setInitParameters(licenseKey);

	// Initialize Vuforia
	int progress = 0;
	while (progress >= 0 && progress < 100)
	{
		progress = Vuforia::init();
		GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Yellow, TEXT("Initializing"));

	}

	if (progress == 100)
	{
		if (Vuforia::isInitialized()) {
			UE_LOG(CFVuforiaLog, Warning, TEXT("AVuforiaActor::Is Initialized"));
			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Green, TEXT("Confirmed that Vuforia is Initialized!"));
		}
		else {
			UE_LOG(CFVuforiaLog, Warning, TEXT("AVuforiaActor::Failed Initialized"));
			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Red, TEXT("Failed Vuforia Initialization"));
		}
	}

	// Failed to initialize Vuforia Engine:
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



}

















void AVuforiaActor::OnCameraActive()
{
	UE_LOG(CFVuforiaLog, Warning, TEXT("AVuforiaActor::On Camera Active"));
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Purple, TEXT("On Camera Active"));

//#if PLATFORM_ANDROID
	static bool once = false;
	if (!once)
	{
		const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
		float ScreenAspect = (float)ViewportSize.X / (float)ViewportSize.Y;
		UE_LOG(CFVuforiaLog, Warning, TEXT("Viewport size = %d,%d"), (int)ViewportSize.X, (int)ViewportSize.Y);

		//const Vuforia::CameraCalibration& calibration = Vuforia::CameraDevice::getInstance().getCameraCalibration();
		//UE_LOG(CFVuforiaLog, Warning, TEXT("CameraActive %f"), calibration.getFieldOfViewRads().data[0] * 180.f / M_PI);
		//GetCameraComponent()->FieldOfView = calibration.getFieldOfViewRads().data[0] * 180.f / M_PI;

		GetCameraComponent()->AspectRatio = ScreenAspect;

		mVideoPlane->Init(GetArPlaneScale());
		once = true;
	}
//#endif
}

// Called every frame
void AVuforiaActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
#if PLATFORM_ANDROID
	
	Vuforia::TrackerManager& trackerManager = Vuforia::TrackerManager::getInstance();
	Vuforia::StateUpdater& stateUpdater = trackerManager.getStateUpdater();
	Vuforia::State state = stateUpdater.updateState();
	// if (state.getNumTrackableResults() == 1)
	if (state.getTrackableResults().size() == 1)
	{
		if (TrackingFound.IsBound())
		{
			TrackingFound.Broadcast();
		}

		const Vuforia::TrackableResult* result = *(state.getTrackableResults().begin());
		//const Vuforia::TrackableResult* result = state.getTrackableResult(0);
		const Vuforia::Trackable& trackable = result->getTrackable();

		Vuforia::Matrix44F mat =
			Vuforia::Tool::convertPose2GLMatrix(result->getPose());

		SetActorTransform(FTransform(CFVuforiaUtils::glToUE(mat)));
	}
	else
	{
		if (TrackingLost.IsBound())
		{
			TrackingLost.Broadcast();
		}
	}
	mVideoPlane->Update();
#endif
}