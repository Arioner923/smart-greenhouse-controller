#pragma once

#include "types.h"

/*
 * Smart Greenhouse Controller
 * Soil Moisture Classification Module Interface
 *
 * Converts the calibrated relative soil-moisture percentage into a
 * descriptive status used by diagnostics and the local dashboard.
 *
 * The percentage is a project-specific relative scale; it is not
 * volumetric water content (VWC).
 */


/*
 * Classify a calibrated relative soil-moisture
 * percentage into a human-readable status.
 */
SoilMoistureStatus classifySoilMoisture(
    float soilMoisturePercent
);

/*
 * Convert a soil-moisture status to text for
 * Serial output and the web dashboard.
 */
const char* soilMoistureStatusToString(
    SoilMoistureStatus status
);