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
 * - Automatic VPD control
 * - Manual humidifier control
 * - Operating mode management
 * - Relay timing protection
 * - Humidity safety cutoff
 * - Invalid-input fail-safe
 */


/* =========================================================
 * Internal Controller State
 * =========================================================
 */

static bool humidifierOn = false;

static bool manualHumidifierRequest = false;

static ControlMode currentControlMode =
    DEFAULT_CONTROL_MODE;

static unsigned long lastRelayStateChangeTime = 0;


/* =========================================================
 * Internal Relay Functions
 * =========================================================
 */


/*
 * Apply a normal relay transition.
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
 * Immediately force the humidifier OFF.
 *
 * Safety conditions and an explicit Manual OFF
 * command are allowed to bypass the minimum ON time.
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
        digitalWrite(
            RELAY_PIN,
            RELAY_OFF_LEVEL
        );
    }
}


static bool canTurnHumidifierOn()
{
    return
        (millis() - lastRelayStateChangeTime) >=
        MIN_RELAY_OFF_TIME_MS;
}


static bool canTurnHumidifierOff()
{
    return
        (millis() - lastRelayStateChangeTime) >=
        MIN_RELAY_ON_TIME_MS;
}


/* =========================================================
 * VPD Profile
 * =========================================================
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

    return VPD_PROFILES[0];
}


/* =========================================================
 * Initialization
 * =========================================================
 */

void initializeControl()
{
    pinMode(RELAY_PIN, OUTPUT);

    digitalWrite(
        RELAY_PIN,
        RELAY_OFF_LEVEL
    );

    humidifierOn = false;
    manualHumidifierRequest = false;

    currentControlMode =
        DEFAULT_CONTROL_MODE;

    lastRelayStateChangeTime = millis();
}


/* =========================================================
 * Operating Mode
 * =========================================================
 */

void setControlMode(ControlMode mode)
{
    if (currentControlMode == mode)
    {
        return;
    }

    /*
     * Safe transition between operating modes.
     */
    forceHumidifierOff();

    manualHumidifierRequest = false;

    currentControlMode = mode;

    /*
     * Restart the minimum OFF timer before the
     * new mode is allowed to activate the relay.
     */
    lastRelayStateChangeTime = millis();
}


ControlMode getControlMode()
{
    return currentControlMode;
}


/* =========================================================
 * Manual Control
 * =========================================================
 */

void setManualHumidifierRequest(bool enabled)
{
    manualHumidifierRequest = enabled;
}


bool getManualHumidifierRequest()
{
    return manualHumidifierRequest;
}


/* =========================================================
 * Main Control Logic
 * =========================================================
 */

ControlState updateControl(
    float currentVPD,
    float currentHumidityPercent,
    bool inputValid
)
{
    ControlState state{};

    state.controlMode =
        currentControlMode;

    state.manualRequestOn =
        manualHumidifierRequest;

    state.humiditySafetyActive = false;
    state.relayTimingLockout = false;


    /*
     * Validate environmental inputs.
     */
    state.controlValid =
        inputValid &&
        !isnan(currentVPD) &&
        !isnan(currentHumidityPercent) &&
        currentHumidityPercent >= 0.0f &&
        currentHumidityPercent <= 100.0f;


    /*
     * FAIL-SAFE PRIORITY 1:
     * Invalid environmental data.
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
     * VPD classification remains available in
     * both AUTO and MANUAL modes for diagnostics.
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
     * Maximum humidity protection.
     */
    if (currentHumidityPercent >=
        MAX_SAFE_HUMIDITY_PERCENT)
    {
        state.humiditySafetyActive = true;

        forceHumidifierOff();

        state.humidifierOn = false;

        return state;
    }


    /* =====================================================
     * MANUAL MODE
     * =====================================================
     */

    if (currentControlMode ==
        ControlMode::Manual)
    {
        /*
         * Manual OFF is immediate.
         *
         * An explicit user OFF command must always
         * be able to stop the humidifier.
         */
        if (!manualHumidifierRequest)
        {
            forceHumidifierOff();
        }

        /*
         * Manual ON still respects the minimum
         * relay OFF time.
         */
        else if (!humidifierOn)
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

        state.humidifierOn =
            humidifierOn;

        return state;
    }


    /* =====================================================
     * AUTOMATIC MODE
     * =====================================================
     */

    /*
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
    else if (currentVPD <=
             profile.targetVPD)
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


    state.humidifierOn =
        humidifierOn;

    return state;
}


/* =========================================================
 * Diagnostic Text
 * =========================================================
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


const char* controlModeToString(ControlMode mode)
{
    switch (mode)
    {
        case ControlMode::Automatic:
            return "AUTO";

        case ControlMode::Manual:
            return "MANUAL";
    }

    return "UNKNOWN";
}