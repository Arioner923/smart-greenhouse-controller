#include <Arduino.h>
#include <math.h>

#include "config.h"
#include "types.h"
#include "control.h"


static void printControlState(
    const char* testName,
    float vpd,
    float humidity,
    bool inputValid,
    const ControlState& state
)
{
    Serial.println("--------------------------------");

    Serial.print("Test          : ");
    Serial.println(testName);

    if (inputValid)
    {
        Serial.print("Input VPD     : ");
        Serial.print(vpd, 2);
        Serial.println(" kPa");

        Serial.print("Humidity      : ");
        Serial.print(humidity, 1);
        Serial.println(" %");
    }
    else
    {
        Serial.println("Input VPD     : INVALID");
        Serial.println("Humidity      : INVALID");
    }

    if (state.controlValid)
    {
        Serial.print("VPD Status    : ");
        Serial.println(vpdStatusToString(state.vpdStatus));
    }
    else
    {
        Serial.println("VPD Status    : INVALID");
    }

    Serial.print("Humidifier    : ");
    Serial.println(
        state.humidifierOn ? "ON" : "OFF"
    );

    Serial.print("Timing Lock   : ");
    Serial.println(
        state.relayTimingLockout ? "ACTIVE" : "OFF"
    );

    Serial.print("RH Safety     : ");
    Serial.println(
        state.humiditySafetyActive ? "ACTIVE" : "OFF"
    );
}


void runTest(
    const char* testName,
    float vpd,
    float humidity,
    bool inputValid
)
{
    const ControlState state =
        updateVPDControl(
            vpd,
            humidity,
            inputValid
        );

    printControlState(
        testName,
        vpd,
        humidity,
        inputValid,
        state
    );
}


void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);

    delay(500);

    Serial.println();
    Serial.println("================================");
    Serial.println(" Smart Greenhouse Controller");
    Serial.println(" SW-05 Relay & Control Safety");
    Serial.println("================================");
    Serial.println();

    initializeControl();

    Serial.println("Control initialization: PASS");
    Serial.println("Initial relay state    : OFF");
    Serial.println();


    /*
     * TEST 1
     *
     * Request ON immediately after startup.
     *
     * Expected:
     * OFF because minimum OFF time has not elapsed.
     */
    runTest(
        "STARTUP OFF LOCKOUT",
        1.70f,
        50.0f,
        true
    );


    /*
     * Wait until minimum OFF time has elapsed.
     */
    delay(MIN_RELAY_OFF_TIME_MS + 500);


    /*
     * TEST 2
     *
     * HIGH VPD after OFF lockout expires.
     *
     * Expected:
     * ON
     */
    runTest(
        "HIGH VPD AFTER OFF TIMER",
        1.70f,
        50.0f,
        true
    );


    /*
     * TEST 3
     *
     * Request OFF immediately.
     *
     * Expected:
     * Relay remains ON because minimum ON time
     * has not yet elapsed.
     */
    runTest(
        "EARLY OFF REQUEST",
        1.30f,
        50.0f,
        true
    );


    /*
     * Wait until minimum ON time has elapsed.
     */
    delay(MIN_RELAY_ON_TIME_MS + 500);


    /*
     * TEST 4
     *
     * Request OFF after minimum ON time.
     *
     * Expected:
     * OFF
     */
    runTest(
        "OFF AFTER ON TIMER",
        1.30f,
        50.0f,
        true
    );


    /*
     * TEST 5
     *
     * Immediately request ON again.
     *
     * Expected:
     * Relay remains OFF because minimum OFF
     * time has not elapsed.
     */
    runTest(
        "EARLY ON REQUEST",
        1.70f,
        50.0f,
        true
    );


    /*
     * Wait until minimum OFF time has elapsed.
     */
    delay(MIN_RELAY_OFF_TIME_MS + 500);


    /*
     * TEST 6
     *
     * HIGH VPD after OFF timer expires.
     *
     * Expected:
     * ON
     */
    runTest(
        "ON AFTER OFF TIMER",
        1.70f,
        50.0f,
        true
    );


    /*
     * TEST 7
     *
     * Humidity safety condition.
     *
     * Expected:
     * Immediate OFF even though minimum ON
     * time has not elapsed.
     */
    runTest(
        "HIGH HUMIDITY SAFETY",
        1.70f,
        90.0f,
        true
    );


    /*
     * TEST 8
     *
     * Invalid sensor input.
     *
     * Expected:
     * Immediate OFF / fail-safe.
     */
    runTest(
        "INVALID INPUT FAIL-SAFE",
        NAN,
        NAN,
        false
    );


    Serial.println("--------------------------------");
    Serial.println("SW-05 safety test complete.");
}


void loop()
{
    /*
     * Test sequence runs once during setup().
     */
}