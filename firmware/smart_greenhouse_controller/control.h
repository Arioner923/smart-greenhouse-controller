#pragma once

#include "types.h"

/*
 * Smart Greenhouse Controller
 * Control Module Interface
 *
 * This module evaluates the current VPD and controls
 * the humidifier relay using the selected growth-stage profile.
 */


/*
 * Initializes the control hardware.
 *
 * The relay is immediately placed in the safe OFF state.
 */
void initializeControl();


/*
 * Updates the VPD control system.
 *
 * Parameters:
 *   currentVPD -> calculated Air VPD in kPa
 *   inputValid -> true only when the environmental measurements
 *                 and calculated VPD are valid
 *
 * Returns:
 *   Current ControlState.
 */
ControlState updateVPDControl(
    float currentVPD,
    float currentHumidityPercent,
    bool inputValid
);


/*
 * Returns the currently selected VPD profile.
 */
const VPDProfile& getActiveVPDProfile();


/*
 * Converts a VPDStatus value into readable text
 * for Serial Monitor diagnostics.
 */
const char* vpdStatusToString(VPDStatus status);