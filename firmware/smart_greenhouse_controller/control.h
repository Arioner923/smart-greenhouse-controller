#pragma once

#include "types.h"

/*
 * Smart Greenhouse Controller
 * Control Module Interface
 */


/*
 * Initialize the control system.
 *
 * The relay always starts in the safe OFF state.
 */
void initializeControl();


/*
 * Change controller operating mode.
 *
 * Changing mode forces the humidifier OFF first
 * to avoid carrying an old relay state into the
 * new operating mode.
 */
void setControlMode(ControlMode mode);


/*
 * Return the currently selected operating mode.
 */
ControlMode getControlMode();

/*
 * Change the active growth stage used by the VPD controller.
 *
 * The new VPD profile becomes effective during the next
 * normal control cycle. Existing relay timing and safety
 * protections remain active.
 */
void setGrowthStage(GrowthStage stage);


/*
 * Return the currently selected runtime growth stage.
 */
GrowthStage getGrowthStage();


/*
 * Set the requested humidifier state while
 * operating in Manual mode.
 *
 * Safety rules still have priority.
 */
void setManualHumidifierRequest(bool enabled);


/*
 * Return the current Manual-mode request.
 */
bool getManualHumidifierRequest();


/*
 * Evaluate the controller and update the relay.
 *
 * In Automatic mode:
 *     VPD determines the requested relay state.
 *
 * In Manual mode:
 *     The user request determines the requested relay state.
 *
 * Safety protections remain active in both modes.
 */
ControlState updateControl(
    float currentVPD,
    float currentHumidityPercent,
    bool inputValid
);


/*
 * Return the currently selected VPD profile.
 */
const VPDProfile& getActiveVPDProfile();


/*
 * Convert diagnostic states to readable text.
 */
const char* vpdStatusToString(VPDStatus status);

const char* controlModeToString(ControlMode mode);