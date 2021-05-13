/*
    Copyright(c) 2016-2017 Codefluegel GMBH All Rights Reserved.
    Distributed under the BSD 3-Clause license.
*/

#pragma once

#include "VuforiaUnrealUWPSettings.generated.h"

UCLASS(Config = Engine, DefaultConfig)
class UVuforiaSettings : public UObject
{
    GENERATED_BODY()

public:
    UVuforiaSettings();
    UVuforiaSettings(FVTableHelper& helper);

    /**
     *  Specifies the License Key to use for this Application
     *
     *  Can be obtained at https://developer.vuforia.com/
     */
    UPROPERTY(Config, EditAnywhere, Category = License)
        FString AppLicenseKey;

};