#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

#include "config.h"
#include "sensors.h"
#include "soil_moisture.h"


/*
 * Smart Greenhouse Controller
 * Sensor Module Implementation
 *
 * This file contains the internal implementation for:
 *
 * - BME280 initialization
 * - Environmental measurements
 * - Soil moisture ADC acquisition
 * - Soil moisture relative percentage conversion
 */


/*
 * BME280 sensor object.
 *
 * This object remains private to the sensor module.
 * Other modules do not need direct access to the BME280 library.
 */
static Adafruit_BME280 bme;


/*
 * Converts a raw soil moisture ADC value to the calibrated
 * relative moisture scale.
 *
 * Calibration:
 *
 *   SOIL_AIR_REFERENCE   -> 0%
 *   SOIL_WATER_REFERENCE -> 100%
 *
 * The output is clamped between 0% and 100%.
 */
static float convertSoilRawToPercent(int rawValue)
{
    float percentage =
        static_cast<float>(SOIL_AIR_REFERENCE - rawValue) /
        static_cast<float>(SOIL_AIR_REFERENCE - SOIL_WATER_REFERENCE)
        * 100.0f;

    if (percentage < 0.0f)
    {
        percentage = 0.0f;
    }

    if (percentage > 100.0f)
    {
        percentage = 100.0f;
    }

    return percentage;
}


/*
 * Initializes the sensor hardware.
 */
bool initializeSensors()
{
    /*
     * Explicitly initialize the ESP8266 I2C bus using the
     * validated shield connections:
     *
     * SDA -> D2 / GPIO4
     * SCL -> D1 / GPIO5
     */
    Wire.begin(D2, D1);

    /*
     * Initialize the BME280 using the validated I2C address.
     */
    if (!bme.begin(BME280_I2C_ADDRESS))
    {
        return false;
    }

    return true;
}


/*
 * Reads all current sensor measurements.
 */
SensorData readSensors()
{
    SensorData data{};

    /*
     * Read environmental measurements from the BME280.
     */
    data.temperature = bme.readTemperature();
    data.humidity = bme.readHumidity();

    /*
     * Adafruit_BME280 returns pressure in Pa.
     * Divide by 100 to convert it to hPa.
     */
    data.pressure = bme.readPressure() / 100.0f;

    /*
     * Read the analog soil sensor.
     */
    data.soilRaw = analogRead(A0);

    /*
     * Convert RAW soil reading to the calibrated relative scale.
     */
    data.soilPercent = convertSoilRawToPercent(data.soilRaw);

    /*
    * Classify the calibrated soil-moisture value.
    */
    data.soilStatus =
        classifySoilMoisture(
            data.soilPercent
        );

    /*
     * Check whether the required environmental measurements
     * contain valid numeric values.
     */
    data.environmentalSensorValid =
        !isnan(data.temperature) &&
        !isnan(data.humidity) &&
        !isnan(data.pressure);

    return data;
}