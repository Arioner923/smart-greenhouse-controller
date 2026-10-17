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

---

# SW-02 - VPD Module Integration

## Objective

Verify that the modular VPD calculation implementation operates correctly
using live environmental measurements from the sensor module.

## Modules Under Test

- `config.h`
- `types.h`
- `sensors.h`
- `sensors.cpp`
- `vpd.h`
- `vpd.cpp`
- `smart_greenhouse_controller.ino`

## Test Procedure

1. Initialize the BME280 through the sensor module.
2. Read live air temperature and relative humidity.
3. Pass the measured values to the VPD calculation module.
4. Calculate Air VPD.
5. Display the result through the Serial Monitor.
6. Verify that the calculated value is physically consistent with the
   measured temperature and humidity.

## Observed Result

    Sensor initialization : PASS
    Temperature           : 24.83 °C
    Humidity              : 58.92 %
    Pressure              : 1009.96 hPa
    Soil RAW              : 739
    Soil Moist.           : 2.0 %
    Air VPD               : 1.29 kPa

## Result

The VPD module successfully received live environmental measurements from
the sensor module and calculated a valid Air Vapor Pressure Deficit.

The observed Air VPD value of 1.29 kPa is consistent with the measured
temperature and relative humidity.

## Status

- [x] PASS
- [ ] FAIL

## Evidence

Serial Monitor screenshot:

    vpd-module-integration-pass.png    