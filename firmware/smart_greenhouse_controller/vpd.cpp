#include <math.h>

#include "vpd.h"


/*
 * Smart Greenhouse Controller
 * VPD Calculation Module Implementation
 *
 * This file contains the mathematical implementation for
 * saturation vapor pressure and Air VPD calculations.
 */


/*
 * Calculate saturation vapor pressure using temperature
 * in degrees Celsius.
 *
 * Result is returned in kPa.
 */
float calculateSaturationVaporPressure(float temperatureC)
{
    return 0.6108f *
           expf(
               (17.27f * temperatureC) /
               (temperatureC + 237.3f)
           );
}


/*
 * Calculate Air Vapor Pressure Deficit from:
 *
 * - Air temperature
 * - Relative humidity
 *
 * Air VPD = SVP(Tair) * (1 - RH / 100)
 */
float calculateAirVPD(
    float temperatureC,
    float relativeHumidityPercent
)
{
    /*
     * Reject invalid input values.
     */
    if (isnan(temperatureC) ||
        isnan(relativeHumidityPercent))
    {
        return NAN;
    }

    if (relativeHumidityPercent < 0.0f ||
        relativeHumidityPercent > 100.0f)
    {
        return NAN;
    }

    const float saturationVaporPressure =
        calculateSaturationVaporPressure(temperatureC);

    return saturationVaporPressure *
           (1.0f - relativeHumidityPercent / 100.0f);
}