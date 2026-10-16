# Hardware Validation Test Plan

This document defines the initial validation procedure for the Smart Greenhouse Controller prototype.

## Test Environment

Controller:

- WeMos D1 R2 V2.1.0
- ESP8266

Connected hardware:

- GY-BME/P280 environmental sensor
- Capacitive soil moisture sensor
- ACEBOTT 5V relay module

Current shield connections:

| Function | WeMos Pin | ESP8266 GPIO |
|---|---|---|
| BME280 SCL | D1 | GPIO5 |
| BME280 SDA | D2 | GPIO4 |
| Soil Sensor Analog | A0 | ADC |
| Relay Signal | D5 | GPIO14 |

---

# Test 01 - ESP8266 Boot Test

## Objective

Verify that the WeMos D1 R2 boots correctly with the prototype shield installed.

## Procedure

1. Disconnect all mains-voltage equipment.
2. Connect the WeMos D1 R2 to the computer through USB.
3. Open the Serial Monitor.
4. Verify that the ESP8266 boots normally.

## Expected Result

The board boots without:

- continuous resets
- abnormal heating
- USB disconnections

## Status

- [x] PASS
- [ ] FAIL

---

# Test 02 - I2C Bus Scan

## Objective

Verify communication between the ESP8266 and the environmental sensor.

## Connections

    D1 / GPIO5 -> SCL
    D2 / GPIO4 -> SDA

## Procedure

1. Upload the I2C scanner firmware.
2. Open Serial Monitor.
3. Scan the I2C bus.

## Expected Result

At least one I2C device should be detected.

Expected BME280/BMP280 address:

- 0x76
or
- 0x77

## Status

- [x] PASS
- [ ] FAIL

---

# Test 03 - Environmental Sensor Identification

## Objective

Determine whether the installed GY-BME/P280 module contains a BME280 or BMP280 sensor.

## Expected Result

The firmware should identify the sensor correctly.

BME280 is preferred because it supports:

- Temperature
- Relative humidity
- Atmospheric pressure

## Status

- [x] PASS
- [ ] FAIL

Detected sensor:

    ______________________

---

# Test 04 - Environmental Measurements

## Objective

Verify valid sensor readings.

## Measurements

- Air temperature
- Relative humidity
- Atmospheric pressure

## Expected Result

Values should be stable and physically reasonable.

## Results

Temperature:

     25.06 - 25.11 °C

Relative Humidity:

    47.85 - 48.35 %

Pressure:

    1008.22 - 1008.26 hPa

## Status

- [x] PASS
- [ ] FAIL

---

# Test 05 - Soil Sensor Raw ADC Test

## Objective

Verify that the ESP8266 can read the analog signal from the capacitive soil moisture sensor.

## Connection

    Soil Analog Output -> A0

## Procedure

1. Read the raw ADC value.
2. Observe the value with the sensor in different moisture conditions.
3. Verify that the reading changes.

## Results

Dry reading:

    __________

Wet reading:

    __________

## Status

- [ ] PASS
- [ ] FAIL

---

# Test 06 - Soil Sensor Calibration

## Objective

Create initial reference values for soil moisture estimation.

## Calibration Points

Dry reference:

    __________

Wet reference:

    __________

## Future Output

The calibrated readings will later be converted to an estimated percentage:

    0%   = Dry reference
    100% = Wet reference

## Status

- [ ] PASS
- [ ] FAIL

---

# Test 07 - Relay Trigger Test

## Objective

Verify control of the ACEBOTT relay module from D5 / GPIO14.

## Safety

No 230V AC load will be connected during this test.

## Procedure

1. Set D5 LOW.
2. Observe relay LED and relay click.
3. Set D5 HIGH.
4. Observe relay LED and relay click.
5. Determine the trigger logic.

## Result

Relay type:

- [ ] Active HIGH
- [ ] Active LOW

## Status

- [ ] PASS
- [ ] FAIL

---

# Test 08 - Combined Hardware Test

## Objective

Verify that all currently installed devices operate correctly at the same time.

## Devices

- BME280
- Soil moisture sensor
- Relay module

## Expected Serial Output

Example:

    Smart Greenhouse Controller

    Temperature : 25.3 °C
    Humidity    : 61.4 %
    Pressure    : 1008.2 hPa
    Soil RAW    : 642
    Relay       : OFF

## Status

- [ ] PASS
- [ ] FAIL

---

# Test 09 - VPD Calculation

## Objective

Calculate Vapor Pressure Deficit using air temperature and relative humidity.

## Requirement

This test requires a confirmed BME280 sensor.

## Expected Output

Example:

    Temperature : 25.0 °C
    Humidity    : 60.0 %
    VPD         : 1.27 kPa

## Status

- [ ] PASS
- [ ] FAIL

---

# Validation Summary

| Test | Status |
|---|---|
| ESP8266 Boot | PASS |
| I2C Scan | PASS |
| Sensor Identification | PASS |
| Environmental Measurements | PASS |
| Soil ADC | Pending |
| Soil Calibration | Pending |
| Relay Trigger | Pending |
| Combined Test | Pending |
| VPD Calculation | Pending |

---

## Notes

Test results, Serial Monitor screenshots and selected hardware photos may be added as validation evidence.

Only verified results should be marked as PASS.