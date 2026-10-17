#pragma once

/*
 * Smart Greenhouse Controller
 * VPD Calculation Module Interface
 *
 * This module contains the mathematical functions used to
 * calculate Vapor Pressure Deficit (VPD).
 */


/*
 * Calculates saturation vapor pressure for a given temperature.
 *
 * Parameter:
 *   temperatureC -> temperature in degrees Celsius
 *
 * Returns:
 *   saturation vapor pressure in kPa
 */
float calculateSaturationVaporPressure(float temperatureC);


/*
 * Calculates Air Vapor Pressure Deficit.
 *
 * Parameters:
 *   temperatureC            -> air temperature in degrees Celsius
 *   relativeHumidityPercent -> relative humidity in percent
 *
 * Returns:
 *   Air VPD in kPa
 *
 * If the input values are invalid, the function returns NAN.
 */
float calculateAirVPD(
    float temperatureC,
    float relativeHumidityPercent
);