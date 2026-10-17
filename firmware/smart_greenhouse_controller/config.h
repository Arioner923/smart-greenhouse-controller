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
 * Control Safety Configuration
 * =========================================================
 *
 * The timing values below are intentionally short during
 * firmware validation so the safety behavior can be tested
 * without long waiting periods.
 *
 * They may be adjusted later when the real humidifier and
 * greenhouse operating conditions are known.
 */


/*
 * Minimum time that the relay must remain ON after activation.
 */
constexpr unsigned long MIN_RELAY_ON_TIME_MS = 5000UL;


/*
 * Minimum time that the relay must remain OFF after deactivation.
 */
constexpr unsigned long MIN_RELAY_OFF_TIME_MS = 5000UL;


/*
 * Initial configurable humidity safety ceiling.
 *
 * This is a controller safety parameter, not a universal
 * recommended humidity value for every crop.
 */
constexpr float MAX_SAFE_HUMIDITY_PERCENT = 85.0f;


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
    GrowthStage::MidLateFlower;


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
 * Operating Mode Configuration
 * =========================================================
 *
 * Default mode after ESP8266 startup.
 *
 * The mode will later be selectable from the web dashboard.
 */

constexpr ControlMode DEFAULT_CONTROL_MODE =
    ControlMode::Automatic;


/* =========================================================
 * Measurement Timing
 * =========================================================
 *
 * Sensors will initially be read every two seconds.
 */

constexpr unsigned long SENSOR_READ_INTERVAL_MS = 2000;

/*
 * Wi-Fi provisioning configuration.
 *
 * These are NOT the user's home Wi-Fi credentials.
 * They define only the temporary setup access point.
 */
constexpr char WIFI_SETUP_AP_NAME[] =
    "SmartGreenhouse-Setup";

constexpr char WIFI_SETUP_AP_PASSWORD[] =
    "Greenhouse123";

/*
 * Maximum time that the configuration portal
 * remains active before returning control
 * to the main firmware.
 */
constexpr unsigned long WIFI_CONFIG_PORTAL_TIMEOUT_SECONDS =
    300UL;

/*
 * Maximum time used for connection attempts.
 */
constexpr unsigned long WIFI_CONNECT_TIMEOUT_SECONDS =
    20UL;