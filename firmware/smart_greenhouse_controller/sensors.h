#pragma once

#include "types.h"

/*
 * Smart Greenhouse Controller
 * Sensor Module Interface
 *
 * This file defines the public interface of the sensor module.
 * Other firmware modules can use these functions without knowing
 * the internal sensor implementation details.
 */


/*
 * Initializes all currently connected sensors.
 *
 * Returns:
 *   true  -> initialization completed successfully
 *   false -> one or more required sensors failed to initialize
 */
bool initializeSensors();


/*
 * Reads all currently supported sensors and returns the results
 * inside a SensorData structure.
 *
 * Current measurements:
 * - Air temperature
 * - Relative humidity
 * - Atmospheric pressure
 * - Soil RAW ADC
 * - Relative soil moisture percentage
 */
SensorData readSensors();