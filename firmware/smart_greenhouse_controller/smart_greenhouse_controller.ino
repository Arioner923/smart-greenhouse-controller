#include <Arduino.h>
#include <math.h>

#include "config.h"
#include "types.h"
#include "sensors.h"
#include "vpd.h"
#include "control.h"


/*
 * Smart Greenhouse Controller
 * Main Application
 *
 * Current firmware capabilities:
 *
 * - Environmental sensor acquisition
 * - Soil moisture measurement
 * - Air VPD calculation
 * - Growth-stage VPD profiles
 * - Automatic humidifier control
 * - Manual control support
 * - Relay timing protection
 * - Maximum humidity safety cutoff
 * - Invalid-input fail-safe
 *
 * Current default operating mode:
 *
 *     AUTO
 *
 * Manual mode will later be controlled through
 * the local web dashboard.
 */


/*
 * Stores the time of the previous measurement/control cycle.
 */
static unsigned long lastMeasurementTime = 0;


/*
 * Print the current complete system state.
 */
static void printSystemStatus(
    const SensorData& data,
    float airVPD,
    const ControlState& controlState
)
{
    const VPDProfile& profile =
        getActiveVPDProfile();

    Serial.println("--------------------------------");

    Serial.print("Mode         : ");
    Serial.println(
        controlModeToString(controlState.controlMode)
    );

    Serial.print("Growth Stage : ");
    Serial.println(profile.name);

    Serial.println();


    /*
     * Environmental measurements
     */
    if (data.environmentalSensorValid)
    {
        Serial.print("Temperature  : ");
        Serial.print(data.temperature, 2);
        Serial.println(" C");

        Serial.print("Humidity     : ");
        Serial.print(data.humidity, 2);
        Serial.println(" %");

        Serial.print("Pressure     : ");
        Serial.print(data.pressure, 2);
        Serial.println(" hPa");
    }
    else
    {
        Serial.println("Environmental sensor data: INVALID");
    }


    Serial.println();


    /*
     * Soil measurements
     */
    Serial.print("Soil RAW     : ");
    Serial.println(data.soilRaw);

    Serial.print("Soil Moist.  : ");
    Serial.print(data.soilPercent, 1);
    Serial.println(" %");


    Serial.println();


    /*
     * VPD information
     */
    if (!isnan(airVPD))
    {
        Serial.print("Air VPD      : ");
        Serial.print(airVPD, 2);
        Serial.println(" kPa");
    }
    else
    {
        Serial.println("Air VPD      : INVALID");
    }

    Serial.print("Target       : ");
    Serial.print(profile.targetVPD, 2);
    Serial.println(" kPa");

    Serial.print("Range        : ");
    Serial.print(profile.minVPD, 2);
    Serial.print(" - ");
    Serial.print(profile.maxVPD, 2);
    Serial.println(" kPa");


    if (controlState.controlValid)
    {
        Serial.print("VPD Status   : ");
        Serial.println(
            vpdStatusToString(controlState.vpdStatus)
        );
    }
    else
    {
        Serial.println("VPD Status   : INVALID");
    }


    Serial.println();


    /*
     * Control status
     */
    if (controlState.controlMode ==
        ControlMode::Manual)
    {
        Serial.print("Manual Req.  : ");
        Serial.println(
            controlState.manualRequestOn ? "ON" : "OFF"
        );
    }

    Serial.print("Humidifier   : ");
    Serial.println(
        controlState.humidifierOn ? "ON" : "OFF"
    );

    Serial.print("Control      : ");
    Serial.println(
        controlState.controlValid ? "ACTIVE" : "FAIL-SAFE"
    );

    Serial.print("Timing Lock  : ");
    Serial.println(
        controlState.relayTimingLockout ? "ACTIVE" : "OFF"
    );

    Serial.print("RH Safety    : ");
    Serial.println(
        controlState.humiditySafetyActive ? "ACTIVE" : "OFF"
    );
}


void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);

    delay(500);

    Serial.println();
    Serial.println("================================");
    Serial.println(" Smart Greenhouse Controller");
    Serial.println(" Main Firmware");
    Serial.println("================================");
    Serial.println();


    /*
     * Initialize the control system first.
     *
     * This immediately places the relay in its
     * safe OFF state.
     */
    initializeControl();


    /*
     * Initialize all connected sensors.
     */
    if (!initializeSensors())
    {
        Serial.println("ERROR: Sensor initialization failed.");
        Serial.println("Automatic control disabled.");
        Serial.println("Humidifier remains OFF.");

        while (true)
        {
            delay(1000);
        }
    }


    Serial.println("Sensor initialization : PASS");
    Serial.println("Control initialization: PASS");

    Serial.println();


    /*
     * Print startup configuration.
     */
    const VPDProfile& profile =
        getActiveVPDProfile();

    Serial.print("Startup Mode          : ");
    Serial.println(
        controlModeToString(getControlMode())
    );

    Serial.print("Selected Profile      : ");
    Serial.println(profile.name);

    Serial.print("Target VPD            : ");
    Serial.print(profile.targetVPD, 2);
    Serial.println(" kPa");

    Serial.print("VPD Range             : ");
    Serial.print(profile.minVPD, 2);
    Serial.print(" - ");
    Serial.print(profile.maxVPD, 2);
    Serial.println(" kPa");

    Serial.print("Maximum Safe RH       : ");
    Serial.print(MAX_SAFE_HUMIDITY_PERCENT, 1);
    Serial.println(" %");

    Serial.println();
}


void loop()
{
    const unsigned long currentTime =
        millis();


    /*
     * Execute one sensor/control cycle at the
     * configured measurement interval.
     */
    if (currentTime - lastMeasurementTime <
        SENSOR_READ_INTERVAL_MS)
    {
        return;
    }

    lastMeasurementTime =
        currentTime;


    /*
     * STEP 1
     *
     * Acquire live sensor measurements.
     */
    const SensorData data =
        readSensors();


    /*
     * STEP 2
     *
     * Calculate Air VPD.
     */
    float airVPD = NAN;

    if (data.environmentalSensorValid)
    {
        airVPD =
            calculateAirVPD(
                data.temperature,
                data.humidity
            );
    }


    /*
     * STEP 3
     *
     * Determine whether the control inputs are valid.
     */
    const bool controlInputValid =
        data.environmentalSensorValid &&
        !isnan(airVPD);


    /*
     * STEP 4
     *
     * Execute AUTO or MANUAL control logic.
     *
     * Both modes still pass through the configured
     * safety protections.
     */
    const ControlState controlState =
        updateControl(
            airVPD,
            data.humidity,
            controlInputValid
        );


    /*
     * STEP 5
     *
     * Print complete diagnostic information.
     */
    printSystemStatus(
        data,
        airVPD,
        controlState
    );
}