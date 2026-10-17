#pragma once

#include <Arduino.h>

#include "types.h"

/*
 * Smart Greenhouse Controller
 * System Configuration
 *
 * This file contains hardware configuration, calibration values
 * and user-selectable operating parameters.
 *
 * Configuration values should be defined here instead of being
 * duplicated throughout the firmware.
 */


/* =========================================================
 * Serial Configuration
 * =========================================================
 */

constexpr unsigned long SERIAL_BAUD_RATE = 115200;


/* =========================================================
 * BME280 Configuration
 * =========================================================
 */

constexpr uint8_t BME280_I2C_ADDRESS = 0x76;


/* =========================================================
 * Soil Moisture Sensor Calibration
 * =========================================================
 *
 * Experimentally measured calibration references:
 *
 * Air   -> approximately 746
 * Water -> approximately 390
 *
 * These values define a relative moisture scale.
 */

constexpr int SOIL_AIR_REFERENCE = 746;
constexpr int SOIL_WATER_REFERENCE = 390;


/* =========================================================
 * Relay Configuration
 * =========================================================
 *
 * Hardware validation confirmed:
 *
 * LOW  -> Relay OFF
 * HIGH -> Relay ON
 */

constexpr uint8_t RELAY_PIN = D5;

constexpr uint8_t RELAY_ON_LEVEL = HIGH;
constexpr uint8_t RELAY_OFF_LEVEL = LOW;


/* =========================================================
 * Growth Stage Selection
 * =========================================================
 *
 * For the first firmware version, the growth stage is selected
 * here during compilation.
 *
 * A future web interface may allow this value to be changed
 * without uploading new firmware.
 * 
 * Select ONE growth stage:
 *
 * GrowthStage::PropagationEarlyVeg
 * GrowthStage::LateVegEarlyFlower
 * GrowthStage::MidLateFlower
 */

constexpr GrowthStage SELECTED_GROWTH_STAGE =
    // GrowthStage::PropagationEarlyVeg;
    GrowthStage::MidLateFlower;
    // GrowthStage::LateVegEarlyFlower;


/* =========================================================
 * VPD Profiles
 * =========================================================
 */

constexpr VPDProfile VPD_PROFILES[] =
{
    {
        "Propagation / Early Veg",
        0.4f,
        0.6f,
        0.8f
    },

    {
        "Late Veg / Early Flower",
        0.8f,
        1.0f,
        1.2f
    },

    {
        "Mid / Late Flower",
        1.2f,
        1.4f,
        1.6f
    }
};


/* =========================================================
 * Measurement Timing
 * =========================================================
 *
 * Sensors will initially be read every two seconds.
 */

constexpr unsigned long SENSOR_READ_INTERVAL_MS = 2000;