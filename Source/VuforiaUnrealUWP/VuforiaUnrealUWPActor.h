/*
	Copyright(c) 2016-2017 Codefluegel GMBH All Rights Reserved.
    Distributed under the BSD 3-Clause license.
*/

#pragma once

#include "Camera/CameraActor.h"

#include "ProceduralMeshComponent.h"

#include "VuforiaUnrealUWPActor.generated.h"


class FViewport;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVuforiaTrackingEvent);

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


	UPROPERTY(BlueprintAssignable, Category = "Vuforia")
	FVuforiaTrackingEvent TrackingFound;

	UPROPERTY(BlueprintAssignable, Category = "Vuforia")
	FVuforiaTrackingEvent TrackingLost;

private:


	constexpr char licenseKey[] = "AVUUtdr/////AAABmVYAZEopnUpJs5SuKrDRvAIvazpGTLd95mB3qRdVTkIUcr9UUu1W9oUBgylfrN4Lu5jFJmuqzxy+m4s6Cs0jZm+0Mh6CqzsVzzjXtyV3iqUjntm0sdjcNKnUnHg0CC+1thEi/YbRrsIcM9n3dHcbGuK/Hy2m1iHKzVFEAxRI3Dj1IUK+c3kMbN3TvJznZw3znw+/MKQq3mNI7jBdOZlBt0TIurwuxQQbLAc8PMz/0lWtTa7vGkY6wt/7k5CE/DYWoccLCP5ULHc6D0vXnzyVfwxiP9l5O5RmiCC5EHwCz8oaWvwmehyOOjplSpIAGh7uJTUFAfg9cm1+PGPwcGl/QIoB3j+SuzmpavvkSlHYwx7M";

	bool initVuforiaInternal();


	FVector2D GetArPlaneScale();

	void OnCameraActive();

	void HideDefaultPawnCollisionComponent();

	class UVuforiaVideoPlaneComponent* mVideoPlane;
};
