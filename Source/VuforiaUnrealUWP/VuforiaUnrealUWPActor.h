/*
	Copyright(c) 2016-2017 Codefluegel GMBH All Rights Reserved.
    Distributed under the BSD 3-Clause license.
*/

#pragma once

// Unreal Engine Files
#include "Core.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraActor.h"

// Vuforia Files
#include <Vuforia/DataSet.h>
#include <Vuforia/CameraDevice.h>

// Standard C Libs
#include <string>

#include "VuforiaUnrealUWPActor.generated.h"


/**
* VuforiaActor positions the camera according to the transformation received from the Vuforia SDK
* and renders the camera stream.
*/
UCLASS(config = DefaultEngine, defaultconfig)
class AVuforiaActor : public ACameraActor
{
	// Sets up the class to support the infrastructure required by the engine.It is required for all UCLASSes.
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	AVuforiaActor();

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Vuforia")
	void BeginInitVuforia();

	UFUNCTION(BlueprintCallable, Category = "Vuforia")
	void BeginCameraVuforia();

private:

	// The currently activated Vuforia DataSet.
	Vuforia::DataSet* currentDataSet = nullptr;

	// The Vuforia camera mode to use, either DEFAULT, SPEED or QUALITY.
	Vuforia::CameraDevice::MODE cameraMode = Vuforia::CameraDevice::MODE_DEFAULT;

	// True when the Vuforia camera is currently started.
	bool isCameraActive = false;

	// True when the Vuforia camera has been started. The camera may currently
	// be stopped because AR has been paused.
	bool isCameraStarted = false;

	// Vuforia
	bool initAR();

	bool initVuforiaInternal();

	bool initTrackers();

	bool loadTrackerData();
	Vuforia::DataSet* loadAndActivateDataSet(std::string path);

	bool startAR();

	bool startTrackers();

	// Vuforia4Unreal Signatures 
	void HideDefaultPawnCollisionComponent();

};
