#include <Arduino.h>
#include <math.h>

#include "config.h"
#include "control.h"


/*
 * Smart Greenhouse Controller
 * Control Module Implementation
 *
 * Responsibilities:
 *
 * - Safe relay initialization
 * - VPD status classification
 * - Growth-stage profile selection
 * - Humidifier ON/OFF control
 * - VPD hysteresis
 * - Minimum relay ON/OFF timing
 * - Maximum humidity safety cutoff
 * - Invalid-input fail-safe behavior
 */


/*
 * Current internal relay state.
 */
static bool humidifierOn = false;


/*
 * Time of the last physical relay state transition.
 *
 * millis() is used so the control system does not need
 * blocking delays during normal operation.
 */
static unsigned long lastRelayStateChangeTime = 0;


/*
 * Apply a new relay state.
 *
 * The transition timestamp is updated only when the
 * physical state actually changes.
 */
static void setHumidifierState(bool enabled)
{
    if (humidifierOn == enabled)
    {
        return;
    }

    humidifierOn = enabled;

    digitalWrite(
        RELAY_PIN,
        enabled ? RELAY_ON_LEVEL : RELAY_OFF_LEVEL
    );

    lastRelayStateChangeTime = millis();
}


/*
 * Force the relay OFF immediately.
 *
 * Safety conditions are allowed to bypass the normal
 * minimum ON-time restriction.
 */
static void forceHumidifierOff()
{
    if (humidifierOn)
    {
        humidifierOn = false;

        digitalWrite(
            RELAY_PIN,
            RELAY_OFF_LEVEL
        );

        lastRelayStateChangeTime = millis();
    }
    else
    {
        /*
         * Ensure the physical output is definitely OFF
         * even when the internal state is already false.
         */
        digitalWrite(
            RELAY_PIN,
            RELAY_OFF_LEVEL
        );
    }
}


/*
 * Check whether the configured minimum OFF time has elapsed.
 */
static bool canTurnHumidifierOn()
{
    return
        (millis() - lastRelayStateChangeTime) >=
        MIN_RELAY_OFF_TIME_MS;
}


/*
 * Check whether the configured minimum ON time has elapsed.
 */
static bool canTurnHumidifierOff()
{
    return
        (millis() - lastRelayStateChangeTime) >=
        MIN_RELAY_ON_TIME_MS;
}


/*
 * Return the VPD profile associated with the selected
 * growth stage.
 */
const VPDProfile& getActiveVPDProfile()
{
    switch (SELECTED_GROWTH_STAGE)
    {
        case GrowthStage::PropagationEarlyVeg:
            return VPD_PROFILES[0];

        case GrowthStage::LateVegEarlyFlower:
            return VPD_PROFILES[1];

        case GrowthStage::MidLateFlower:
            return VPD_PROFILES[2];
    }

    /*
     * Defensive fallback.
     */
    return VPD_PROFILES[0];
}


/*
 * Initialize control hardware.
 */
void initializeControl()
{
    pinMode(RELAY_PIN, OUTPUT);

    /*
     * Safe startup state.
     */
    digitalWrite(
        RELAY_PIN,
        RELAY_OFF_LEVEL
    );

    humidifierOn = false;

    /*
     * Starting this timer here also provides a short
     * startup OFF period before automatic activation.
     */
    lastRelayStateChangeTime = millis();
}


/*
 * Update automatic VPD control and apply safety rules.
 */
ControlState updateVPDControl(
    float currentVPD,
    float currentHumidityPercent,
    bool inputValid
)
{
    ControlState state{};

    state.humiditySafetyActive = false;
    state.relayTimingLockout = false;


    /*
     * Validate control inputs.
     */
    state.controlValid =
        inputValid &&
        !isnan(currentVPD) &&
        !isnan(currentHumidityPercent) &&
        currentHumidityPercent >= 0.0f &&
        currentHumidityPercent <= 100.0f;


    /*
     * FAIL-SAFE PRIORITY 1:
     *
     * Invalid environmental data immediately disables
     * the humidifier.
     */
    if (!state.controlValid)
    {
        forceHumidifierOff();

        state.vpdStatus = VPDStatus::Low;
        state.humidifierOn = false;

        return state;
    }


    const VPDProfile& profile =
        getActiveVPDProfile();


    /*
     * Classify VPD condition.
     */
    if (currentVPD < profile.minVPD)
    {
        state.vpdStatus = VPDStatus::Low;
    }
    else if (currentVPD > profile.maxVPD)
    {
        state.vpdStatus = VPDStatus::High;
    }
    else
    {
        state.vpdStatus = VPDStatus::InRange;
    }


    /*
     * FAIL-SAFE PRIORITY 2:
     *
     * Excessive humidity immediately disables the humidifier.
     *
     * This safety action intentionally bypasses the minimum
     * relay ON-time restriction.
     */
    if (currentHumidityPercent >=
        MAX_SAFE_HUMIDITY_PERCENT)
    {
        state.humiditySafetyActive = true;

        forceHumidifierOff();

        state.humidifierOn = false;

        return state;
    }


    /*
     * NORMAL VPD CONTROL
     *
     * HIGH VPD:
     * Request humidifier ON.
     */
    if (currentVPD > profile.maxVPD)
    {
        if (!humidifierOn)
        {
            if (canTurnHumidifierOn())
            {
                setHumidifierState(true);
            }
            else
            {
                state.relayTimingLockout = true;
            }
        }
    }

    /*
     * Target reached:
     * Request humidifier OFF.
     */
    else if (currentVPD <= profile.targetVPD)
    {
        if (humidifierOn)
        {
            if (canTurnHumidifierOff())
            {
                setHumidifierState(false);
            }
            else
            {
                state.relayTimingLockout = true;
            }
        }
    }


    state.humidifierOn = humidifierOn;

    return state;
}


/*
 * Convert VPD status to readable text.
 */
const char* vpdStatusToString(VPDStatus status)
{
    switch (status)
    {
        case VPDStatus::Low:
            return "LOW";

        case VPDStatus::InRange:
            return "IN RANGE";

        case VPDStatus::High:
            return "HIGH";
    }

    return "UNKNOWN";
}