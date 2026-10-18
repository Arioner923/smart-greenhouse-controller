#include "soil_moisture.h"

#include "config.h"


/*
 * Smart Greenhouse Controller
 * Soil Moisture Classification Module Implementation
 *
 * Classification boundaries are defined centrally in config.h. The module
 * contains no irrigation logic; it only translates the calibrated relative
 * moisture value into a reusable status.
 */


SoilMoistureStatus classifySoilMoisture(
    float soilMoisturePercent
)
{
    if (soilMoisturePercent <
        SOIL_VERY_DRY_MAX_PERCENT)
    {
        return SoilMoistureStatus::VeryDry;
    }


    if (soilMoisturePercent <
        SOIL_DRY_MAX_PERCENT)
    {
        return SoilMoistureStatus::Dry;
    }


    if (soilMoisturePercent <
        SOIL_GOOD_MAX_PERCENT)
    {
        return SoilMoistureStatus::Good;
    }


    if (soilMoisturePercent <
        SOIL_WET_MAX_PERCENT)
    {
        return SoilMoistureStatus::Wet;
    }


    return SoilMoistureStatus::VeryWet;
}


const char* soilMoistureStatusToString(
    SoilMoistureStatus status
)
{
    switch (status)
    {
        case SoilMoistureStatus::VeryDry:
            return "VERY DRY";

        case SoilMoistureStatus::Dry:
            return "DRY";

        case SoilMoistureStatus::Good:
            return "GOOD";

        case SoilMoistureStatus::Wet:
            return "WET";

        case SoilMoistureStatus::VeryWet:
            return "VERY WET";

        default:
            return "UNKNOWN";
    }
}