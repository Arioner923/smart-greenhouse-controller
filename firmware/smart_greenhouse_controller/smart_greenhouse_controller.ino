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
 * Current data flow:
 *
 * Sensors
 *    ↓
 * SensorData
 *    ↓
 * Air VPD
 *    ↓
 * Growth-stage profile
 *    ↓
 * Control logic
 *    ↓
 * Humidifier relay
 */


/*
 * Stores the time of the previous measurement cycle.
 *
 * This allows the controller to read the sensors at the
 * configured interval without continuously flooding the
 * Serial Monitor.
 */
static unsigned long lastMeasurementTime = 0;


/*
 * Print the current system state to the Serial Monitor.
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

    Serial.print("Growth Stage : ");
    Serial.println(profile.name);

    Serial.println();

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

    Serial.print("Soil RAW     : ");
    Serial.println(data.soilRaw);

    Serial.print("Soil Moist.  : ");
    Serial.print(data.soilPercent, 1);
    Serial.println(" %");

    Serial.println();

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

    Serial.print("Humidifier   : ");
    Serial.println(
        controlState.humidifierOn ? "ON" : "OFF"
    );

    Serial.print("Control      : ");
    Serial.println(
        controlState.controlValid ? "ACTIVE" : "FAIL-SAFE"
    );
}


void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);

    delay(500);

    Serial.println();
    Serial.println("================================");
    Serial.println(" Smart Greenhouse Controller");
    Serial.println(" Automatic VPD Control");
    Serial.println("================================");
    Serial.println();


    /*
     * IMPORTANT:
     *
     * Initialize the control system first.
     *
     * This immediately configures the relay output and
     * places the humidifier in the safe OFF state before
     * sensor initialization begins.
     */
    initializeControl();


    /*
     * Initialize the connected sensors.
     */
    if (!initializeSensors())
    {
        Serial.println("ERROR: Sensor initialization failed.");
        Serial.println("Automatic VPD control disabled.");
        Serial.println("Humidifier remains OFF.");

        /*
         * The relay has already been initialized OFF.
         *
         * Stop normal execution because automatic control
         * cannot operate safely without the environmental sensor.
         */
        while (true)
        {
            delay(1000);
        }
    }


    Serial.println("Sensor initialization : PASS");
    Serial.println("Control initialization: PASS");

    Serial.println();

    const VPDProfile& profile =
        getActiveVPDProfile();

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

    Serial.println();
}


void loop()
{
    const unsigned long currentTime = millis();

    /*
     * Execute one measurement/control cycle at the
     * configured interval.
     */
    if (currentTime - lastMeasurementTime <
        SENSOR_READ_INTERVAL_MS)
    {
        return;
    }

    lastMeasurementTime = currentTime;


    /*
     * STEP 1:
     * Acquire real sensor measurements.
     */
    const SensorData data =
        readSensors();


    /*
     * STEP 2:
     * Calculate Air VPD only when the environmental
     * measurements are valid.
     */
    float airVPD = NAN;

    if (data.environmentalSensorValid)
    {
        airVPD = calculateAirVPD(
            data.temperature,
            data.humidity
        );
    }


    /*
     * STEP 3:
     * Determine whether the control input is valid.
     */
    const bool controlInputValid =
        data.environmentalSensorValid &&
        !isnan(airVPD);


    /*
     * STEP 4:
     * Evaluate the selected VPD profile and automatically
     * update the humidifier relay.
     */
    const ControlState controlState =
        updateVPDControl(
            airVPD,
            controlInputValid
        );


    /*
     * STEP 5:
     * Print the complete system state.
     */
    printSystemStatus(
        data,
        airVPD,
        controlState
    );
}