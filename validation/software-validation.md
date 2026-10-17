# Software Validation

This document records validation results for the modular Smart Greenhouse
Controller firmware.

The purpose of these tests is to verify that individual software modules
operate correctly when integrated with the validated greenhouse hardware.

---

# SW-01 - Sensor Module Integration

## Objective

Verify that the modular sensor implementation operates correctly when integrated
with the main Smart Greenhouse Controller firmware.

## Modules Under Test

- `config.h`
- `types.h`
- `sensors.h`
- `sensors.cpp`
- `smart_greenhouse_controller.ino`

## Hardware

Controller:

- WeMos D1 R2 V2.1.0
- ESP8266

Sensors:

- BME280
- Capacitive soil moisture sensor

## Test Procedure

1. Upload the modular Smart Greenhouse Controller firmware.
2. Open the Serial Monitor at 115200 baud.
3. Verify successful sensor initialization.
4. Observe BME280 environmental measurements.
5. Observe the soil sensor RAW ADC value.
6. Verify conversion of the RAW soil value to the calibrated relative
   moisture percentage.

## Expected Result

The firmware should:

- Initialize the BME280 successfully.
- Read valid air temperature.
- Read valid relative humidity.
- Read valid atmospheric pressure.
- Read the soil sensor ADC value.
- Convert the soil RAW reading to the configured relative moisture percentage.
- Report valid sensor data through the Serial Monitor.

## Observed Result

    Sensor initialization : PASS
    Temperature           : 24.52 °C
    Humidity              : 59.76 %
    Pressure              : 1010.02 hPa
    Soil RAW              : 738
    Soil Moist.           : 2.2 %

## Soil Moisture Verification

Configured calibration references:

    Air reference   : 746
    Water reference : 390

Observed RAW value:

    738

Calculated relative soil moisture:

    2.2 %

The calculated percentage is consistent with the configured calibration
references and confirms that the soil conversion function operates correctly.

## Result

The modular sensor implementation successfully initialized the BME280 and
produced valid environmental and soil measurements.

Communication between the main firmware and the sensor module operated as
expected.

## Status

- [x] PASS
- [ ] FAIL

## Evidence

Serial Monitor screenshot:

    sensor-module-integration-pass.png