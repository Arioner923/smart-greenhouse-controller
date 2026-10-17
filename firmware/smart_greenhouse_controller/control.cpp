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
 * - Fail-safe behavior
 */


/*
 * Internal humidifier state.
 *
 * The variable is private to this module.
 */
static bool humidifierOn = false;


/*
 * Apply the requested humidifier state to the physical relay.
 */
static void setHumidifierState(bool enabled)
{
    humidifierOn = enabled;

    digitalWrite(
        RELAY_PIN,
        enabled ? RELAY_ON_LEVEL : RELAY_OFF_LEVEL
    );
}


/*
 * Return the VPD profile associated with the
 * currently selected growth stage.
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
     *
     * This should never normally be reached.
     */
    return VPD_PROFILES[0];
}


/*
 * Initialize the relay output in the safe OFF state.
 */
void initializeControl()
{
    pinMode(RELAY_PIN, OUTPUT);

    setHumidifierState(false);
}


/*
 * Update automatic VPD control.
 */
ControlState updateVPDControl(
    float currentVPD,
    bool inputValid
)
{
    ControlState state{};

    /*
     * Fail-safe behavior.
     *
     * Automatic control is disabled if the environmental
     * measurements or calculated VPD are invalid.
     */
    state.controlValid =
        inputValid &&
        !isnan(currentVPD);

    if (!state.controlValid)
    {
        setHumidifierState(false);

        state.vpdStatus = VPDStatus::Low;
        state.humidifierOn = false;

        return state;
    }


    const VPDProfile& profile =
        getActiveVPDProfile();


    /*
     * Classify the measured VPD.
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
     * Humidifier control with hysteresis.
     *
     * HIGH VPD:
     *     Turn humidifier ON.
     *
     * VPD reaches target:
     *     Turn humidifier OFF.
     *
     * Between target and maximum:
     *     Keep the previous relay state.
     */
    if (currentVPD > profile.maxVPD)
    {
        setHumidifierState(true);
    }
    else if (currentVPD <= profile.targetVPD)
    {
        setHumidifierState(false);
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