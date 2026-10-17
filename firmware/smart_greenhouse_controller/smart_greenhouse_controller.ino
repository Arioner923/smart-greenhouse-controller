#include <Arduino.h>

#include "config.h"
#include "types.h"
#include "control.h"


void runControlTest(
    const char* testName,
    float testVPD,
    bool inputValid
)
{
    const VPDProfile& profile =
        getActiveVPDProfile();

    ControlState state =
        updateVPDControl(testVPD, inputValid);

    Serial.println("--------------------------------");

    Serial.print("Test          : ");
    Serial.println(testName);

    if (inputValid)
    {
        Serial.print("Input VPD     : ");
        Serial.print(testVPD, 2);
        Serial.println(" kPa");
    }
    else
    {
        Serial.println("Input VPD     : INVALID");
    }

    Serial.print("Profile       : ");
    Serial.println(profile.name);

    Serial.print("Target        : ");
    Serial.print(profile.targetVPD, 2);
    Serial.println(" kPa");

    Serial.print("Range         : ");
    Serial.print(profile.minVPD, 2);
    Serial.print(" - ");
    Serial.print(profile.maxVPD, 2);
    Serial.println(" kPa");

    if (state.controlValid)
    {
        Serial.print("VPD Status    : ");
        Serial.println(
            vpdStatusToString(state.vpdStatus)
        );
    }
    else
    {
        Serial.println("VPD Status    : INVALID");
    }

    Serial.print("Humidifier    : ");
    Serial.println(
        state.humidifierOn ? "ON" : "OFF"
    );

    delay(3000);
}


void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);

    delay(500);

    Serial.println();
    Serial.println("================================");
    Serial.println(" Smart Greenhouse Controller");
    Serial.println(" Control Module Integration Test");
    Serial.println("================================");
    Serial.println();

    /*
     * IMPORTANT:
     *
     * Control is initialized before any automatic
     * decisions so that the relay starts safely OFF.
     */
    initializeControl();

    const VPDProfile& profile =
        getActiveVPDProfile();

    Serial.print("Selected Profile: ");
    Serial.println(profile.name);

    Serial.println();


    /*
     * Test 1:
     *
     * VPD above maximum.
     *
     * Expected:
     * HIGH / Humidifier ON
     */
    runControlTest(
        "HIGH VPD",
        1.70f,
        true
    );


    /*
     * Test 2:
     *
     * VPD inside the range but still above target.
     *
     * The humidifier should remain ON because it was
     * activated during Test 1.
     *
     * This validates hysteresis.
     */
    runControlTest(
        "HYSTERESIS HOLD",
        1.50f,
        true
    );


    /*
     * Test 3:
     *
     * VPD reaches target.
     *
     * Expected:
     * Humidifier OFF
     */
    runControlTest(
        "TARGET REACHED",
        1.40f,
        true
    );


    /*
     * Test 4:
     *
     * VPD below minimum.
     *
     * Expected:
     * LOW / Humidifier OFF
     */
    runControlTest(
        "LOW VPD",
        1.10f,
        true
    );


    /*
     * Test 5:
     *
     * Invalid environmental input.
     *
     * Expected:
     * Fail-safe relay OFF
     */
    runControlTest(
        "INVALID INPUT",
        NAN,
        false
    );


    Serial.println("--------------------------------");
    Serial.println("Control test sequence complete.");
}


void loop()
{
    /*
     * No repeated test cycle is required.
     */
}