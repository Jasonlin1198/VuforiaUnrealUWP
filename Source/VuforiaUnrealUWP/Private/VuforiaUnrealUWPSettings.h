#pragma once

#include "VuforiaUnrealUWPSettings.generated.h"

UCLASS(Config = Engine, DefaultConfig)
class UVuforiaSettings : public UObject
{
    GENERATED_BODY()

public:
    UVuforiaSettings();
    UVuforiaSettings(FVTableHelper& helper);

    UPROPERTY(Config, EditAnywhere, Category = License)
        FString VuforiaLicenseKey;

};