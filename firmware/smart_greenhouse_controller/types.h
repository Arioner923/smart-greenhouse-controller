#pragma once

/*
 * Smart Greenhouse Controller
 * Shared Data Types
 *
 * This file contains common enums and structures that can be used
 * by multiple firmware modules.
 */


/*
 * Growth stage selected by the user.
 *
 * Each growth stage will be associated with a different VPD profile.
 */
enum class GrowthStage
{
    PropagationEarlyVeg,
    LateVegEarlyFlower,
    MidLateFlower
};


/*
 * VPD operating profile.
 *
 * minVPD:
 *     Lower boundary of the recommended VPD range.
 *
 * targetVPD:
 *     Desired operating point.
 *
 * maxVPD:
 *     Upper boundary of the recommended VPD range.
 */
struct VPDProfile
{
    const char* name;

    float minVPD;
    float targetVPD;
    float maxVPD;
};


/*
 * Environmental and soil measurements collected by the sensor module.
 *
 * environmentalSensorValid is used by the control system so that
 * automatic operation can be disabled if the BME280 reading fails.
 */
struct SensorData
{
    float temperature;
    float humidity;
    float pressure;

    int soilRaw;
    float soilPercent;

    bool environmentalSensorValid;
};


/*
 * Current VPD condition compared with the selected VPD profile.
 */
enum class VPDStatus
{
    Low,
    InRange,
    High
};