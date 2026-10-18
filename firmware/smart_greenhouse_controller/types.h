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
 * Relative soil-moisture classification.
 *
 * The percentage is based on the calibrated relative
 * sensor scale and does not represent volumetric
 * water content (VWC).
 */
enum class SoilMoistureStatus
{
    VeryDry,
    Dry,
    Good,
    Wet,
    VeryWet
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
    SoilMoistureStatus soilStatus;

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


/*
 * Operating mode of the greenhouse controller.
 *
 * Automatic:
 *     Humidifier state is controlled from VPD.
 *
 * Manual:
 *     Humidifier state is requested by the user.
 *     Safety protections remain active.
 */
enum class ControlMode
{
    Automatic,
    Manual
};

/*
 * Current output state of the VPD control system.
 *
 * controlValid:
 *     Indicates whether automatic control is allowed to operate.
 *
 * humidifierOn:
 *     Current commanded state of the humidifier relay.
 */
struct ControlState
{
    VPDStatus vpdStatus;
    ControlMode controlMode;

    bool humidifierOn;
    bool controlValid;

    bool humiditySafetyActive;
    bool relayTimingLockout;

    /*
     * Requested humidifier state while operating
     * in Manual mode.
     */
    bool manualRequestOn;
};