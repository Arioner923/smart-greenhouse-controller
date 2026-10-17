#include <Arduino.h>
#include <math.h>
#include <ESP8266WiFi.h>

#include "config.h"
#include "types.h"
#include "sensors.h"
#include "vpd.h"
#include "control.h"
#include "wifi_provisioning.h"


/*
 * Smart Greenhouse Controller
 * Main Application
 *
 * Current capabilities:
 *
 * - Environmental measurements
 * - Soil moisture monitoring
 * - Air VPD calculation
 * - Growth-stage VPD profiles
 * - AUTO / MANUAL operation
 * - Relay timing protection
 * - Maximum humidity safety cutoff
 * - Invalid-input fail-safe
 * - Wi-Fi provisioning
 * - Persistent Wi-Fi credentials
 * - Offline greenhouse operation
 */


static unsigned long lastMeasurementTime = 0;


/*
 * Prints the complete current greenhouse state.
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
     * Environmental measurements.
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
        Serial.println(
            "Environmental sensor data: INVALID"
        );
    }


    Serial.println();


    /*
     * Soil measurements.
     */
    Serial.print("Soil RAW     : ");
    Serial.println(data.soilRaw);

    Serial.print("Soil Moist.  : ");
    Serial.print(data.soilPercent, 1);
    Serial.println(" %");


    Serial.println();


    /*
     * VPD information.
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
     * Control status.
     */
    if (controlState.controlMode ==
        ControlMode::Manual)
    {
        Serial.print("Manual Req.  : ");
        Serial.println(
            controlState.manualRequestOn
                ? "ON"
                : "OFF"
        );
    }


    Serial.print("Humidifier   : ");
    Serial.println(
        controlState.humidifierOn
            ? "ON"
            : "OFF"
    );

    Serial.print("Control      : ");
    Serial.println(
        controlState.controlValid
            ? "ACTIVE"
            : "FAIL-SAFE"
    );

    Serial.print("Timing Lock  : ");
    Serial.println(
        controlState.relayTimingLockout
            ? "ACTIVE"
            : "OFF"
    );

    Serial.print("RH Safety    : ");
    Serial.println(
        controlState.humiditySafetyActive
            ? "ACTIVE"
            : "OFF"
    );


    Serial.println();


    /*
     * Network status.
     *
     * Wi-Fi is informational only.
     * Greenhouse control does not depend on it.
     */
    Serial.print("Wi-Fi        : ");

    if (isWiFiConnected())
    {
        Serial.println("CONNECTED");

        Serial.print("IP Address   : ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("OFFLINE");
    }
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
     * STEP 1
     *
     * Initialize the control system first.
     *
     * Relay starts in the safe OFF state.
     */
    initializeControl();


    /*
     * STEP 2
     *
     * Initialize sensors.
     */
    const bool sensorsReady =
        initializeSensors();

    Serial.print("Sensor initialization : ");
    Serial.println(
        sensorsReady ? "PASS" : "FAIL"
    );

    Serial.println(
        "Control initialization: PASS"
    );


    /*
     * STEP 3
     *
     * Initialize Wi-Fi provisioning.
     *
     * Wi-Fi failure must never disable
     * greenhouse monitoring/control.
     */
    const bool wifiConnected =
        initializeWiFiProvisioning();

    Serial.print("Wi-Fi initialization  : ");

    if (wifiConnected)
    {
        Serial.println("CONNECTED");
    }
    else
    {
        Serial.println("OFFLINE / SETUP MODE");
    }


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
    Serial.print(
        MAX_SAFE_HUMIDITY_PERCENT,
        1
    );
    Serial.println(" %");

    Serial.println();
}


void loop()
{
    /*
     * Keep Wi-Fi provisioning responsive.
     *
     * This is non-blocking.
     */
    processWiFiProvisioning();


    const unsigned long currentTime =
        millis();


    /*
     * Run greenhouse monitoring/control at
     * the configured measurement interval.
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
     * Read sensors.
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
     * Determine control-input validity.
     */
    const bool controlInputValid =
        data.environmentalSensorValid &&
        !isnan(airVPD);


    /*
     * STEP 4
     *
     * Execute AUTO or MANUAL control.
     *
     * Safety protections remain active
     * regardless of Wi-Fi state.
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
     * Print system diagnostics.
     */
    printSystemStatus(
        data,
        airVPD,
        controlState
    );
}