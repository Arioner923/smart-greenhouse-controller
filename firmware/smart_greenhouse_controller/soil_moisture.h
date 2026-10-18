#pragma once

#include "types.h"

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