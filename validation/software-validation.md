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

---

# SW-03 - Control Module Integration

## Objective

Verify that the VPD control module correctly evaluates VPD conditions,
applies hysteresis and controls the humidifier relay safely.

## Modules Under Test

- `config.h`
- `types.h`
- `control.h`
- `control.cpp`
- `smart_greenhouse_controller.ino`

## Selected Profile

    Mid / Late Flower

Configured values:

    Minimum VPD : 1.20 kPa
    Target VPD  : 1.40 kPa
    Maximum VPD : 1.60 kPa

## Test Cases

### High VPD

    Input VPD  : 1.70 kPa
    VPD Status : HIGH
    Humidifier : ON

### Hysteresis Hold

    Input VPD  : 1.50 kPa
    VPD Status : IN RANGE
    Humidifier : ON

The relay remained ON because VPD had not yet reached the configured target.

### Target Reached

    Input VPD  : 1.40 kPa
    VPD Status : IN RANGE
    Humidifier : OFF

The relay switched OFF when the target VPD was reached.

### Low VPD

    Input VPD  : 1.10 kPa
    VPD Status : LOW
    Humidifier : OFF

### Invalid Input

    Input VPD  : INVALID
    VPD Status : INVALID
    Humidifier : OFF

The fail-safe logic correctly forced the relay to the OFF state.

## Result

The control module correctly:

- Classified VPD as LOW, IN RANGE or HIGH.
- Activated the humidifier above the maximum VPD threshold.
- Preserved the previous relay state inside the hysteresis region.
- Deactivated the humidifier when the target VPD was reached.
- Kept the humidifier OFF during LOW VPD.
- Forced the humidifier OFF when control input was invalid.

## Status

- [x] PASS
- [ ] FAIL

## Evidence

Serial Monitor screenshots:

    control-module-integration-pass.png

---

# SW-04 - Automatic VPD Control Integration

## Objective

Verify the complete automatic VPD control data flow using live environmental
sensor measurements.

The test validates the full path:

    BME280
      ↓
    Sensor Module
      ↓
    Air VPD Calculation
      ↓
    Growth-Stage Profile
      ↓
    Control Logic
      ↓
    Humidifier Relay

## Hardware

- WeMos D1 R2 V2.1.0 / ESP8266
- BME280
- Capacitive soil moisture sensor
- ACEBOTT 5V relay module

No 230V AC load was connected during this validation.

## Test 1 - In-Range VPD

Selected profile:

    Mid / Late Flower

Configured profile:

    Minimum VPD : 1.20 kPa
    Target VPD  : 1.40 kPa
    Maximum VPD : 1.60 kPa

Observed values:

    Temperature : 24.91 °C
    Humidity    : 58.79 %
    Pressure    : 1010.16 hPa
    Soil RAW    : 737
    Soil Moist. : 2.5 %
    Air VPD     : 1.30 kPa

Controller output:

    VPD Status  : IN RANGE
    Humidifier  : OFF
    Control     : ACTIVE

The controller correctly kept the humidifier OFF because the measured
Air VPD was inside the selected profile range.

## Test 2 - High VPD

For validation, the selected growth-stage profile was temporarily changed to:

    Propagation / Early Veg

Configured profile:

    Minimum VPD : 0.40 kPa
    Target VPD  : 0.60 kPa
    Maximum VPD : 0.80 kPa

Observed values:

    Temperature : 24.92 °C
    Humidity    : 58.95 %
    Pressure    : 1010.22 hPa
    Soil RAW    : 738
    Soil Moist. : 2.2 %
    Air VPD     : 1.29 kPa

Controller output:

    VPD Status  : HIGH
    Humidifier  : ON
    Control     : ACTIVE

The controller correctly identified that the measured Air VPD exceeded
the selected profile maximum and automatically activated the humidifier relay.

## Result

The complete automatic VPD control chain operated successfully using live
sensor measurements.

The firmware successfully:

- Acquired live environmental measurements.
- Calculated Air VPD.
- Applied the selected growth-stage profile.
- Classified the current VPD condition.
- Kept the humidifier OFF when VPD was inside the selected range.
- Activated the humidifier automatically when VPD exceeded the maximum limit.
- Maintained active automatic control using real sensor data.

## Status

- [x] PASS
- [ ] FAIL

## Evidence

Serial Monitor screenshots:

    automatic-vpd-control-in-range-pass.png

---

# SW-05 - Relay and Control Safety

## Objective

Verify the relay timing protections and control safety mechanisms before
connecting a real humidifier.

## Safety Features Under Test

- Safe relay startup
- Minimum relay OFF time
- Minimum relay ON time
- Humidity safety cutoff
- Invalid-input fail-safe

## Configuration

Validation timing values:

    Minimum ON time  : 5000 ms
    Minimum OFF time : 5000 ms

Humidity safety limit:

    Maximum RH : 85.0 %

The relay timing values are intentionally short for firmware validation and
may be adjusted later when the real humidifier is available.

## Test Results

### Startup OFF Lockout

    Input VPD     : 1.70 kPa
    Humidity      : 50.0 %
    VPD Status    : HIGH
    Humidifier    : OFF
    Timing Lock   : ACTIVE
    RH Safety     : OFF

The relay remained OFF immediately after startup because the minimum OFF time
had not yet elapsed.

### HIGH VPD After OFF Timer

    Input VPD     : 1.70 kPa
    Humidity      : 50.0 %
    VPD Status    : HIGH
    Humidifier    : ON
    Timing Lock   : OFF

The relay activated after the minimum OFF time elapsed.

### Early OFF Request

    Input VPD     : 1.30 kPa
    Humidity      : 50.0 %
    VPD Status    : IN RANGE
    Humidifier    : ON
    Timing Lock   : ACTIVE

The relay remained ON because the minimum ON time had not yet elapsed.

### OFF After ON Timer

    Input VPD     : 1.30 kPa
    Humidity      : 50.0 %
    VPD Status    : IN RANGE
    Humidifier    : OFF
    Timing Lock   : OFF

The relay switched OFF after the minimum ON time elapsed.

### Early ON Request

    Input VPD     : 1.70 kPa
    Humidity      : 50.0 %
    VPD Status    : HIGH
    Humidifier    : OFF
    Timing Lock   : ACTIVE

The relay remained OFF because the minimum OFF time had not yet elapsed.

### ON After OFF Timer

    Input VPD     : 1.70 kPa
    Humidity      : 50.0 %
    VPD Status    : HIGH
    Humidifier    : ON
    Timing Lock   : OFF

The relay activated after the minimum OFF time elapsed.

### High Humidity Safety

    Input VPD     : 1.70 kPa
    Humidity      : 90.0 %
    VPD Status    : HIGH
    Humidifier    : OFF
    Timing Lock   : OFF
    RH Safety     : ACTIVE

The humidity safety system correctly overrode the VPD control request and
forced the humidifier OFF.

### Invalid Input Fail-Safe

    Input VPD     : INVALID
    Humidity      : INVALID
    VPD Status    : INVALID
    Humidifier    : OFF
    Timing Lock   : OFF
    RH Safety     : OFF

The controller correctly entered fail-safe behavior and kept the relay OFF.

## Result

The relay and control safety protections operated correctly.

The firmware successfully validated:

- Safe OFF startup
- Minimum relay ON time
- Minimum relay OFF time
- Timing lockout behavior
- Maximum humidity safety override
- Invalid sensor input fail-safe behavior

## Status

- [x] PASS
- [ ] FAIL

## Evidence

Serial Monitor screenshot:

    relay-control-safety-pass.png

---

# SW-06 - AUTO / MANUAL Operating Modes

## Objective

Verify that the greenhouse controller can safely switch between Automatic
and Manual operating modes while preserving all control safety protections.

## Operating Modes

### Automatic Mode

The humidifier is controlled automatically using:

    Sensor Data
        ↓
    Air VPD
        ↓
    Growth-Stage Profile
        ↓
    Control Logic
        ↓
    Relay

### Manual Mode

The humidifier state is requested by the user.

Manual control does not bypass:

- Invalid-input fail-safe
- Maximum humidity safety cutoff
- Minimum relay OFF time

An explicit Manual OFF command is allowed to stop the humidifier immediately.

## Test Results

### Automatic HIGH VPD

    Mode           : AUTO
    Input VPD      : 1.70 kPa
    Humidity       : 50.0 %
    VPD Status     : HIGH
    Humidifier     : ON

Automatic VPD control activated the relay correctly.

### Switch to Manual Mode

    Mode           : MANUAL
    Manual Request : OFF
    Humidifier     : OFF

Changing operating mode safely forced the relay OFF.

### Manual ON Lockout

    Mode           : MANUAL
    Manual Request : ON
    Humidifier     : OFF
    Timing Lock    : ACTIVE

The manual ON request was temporarily blocked because the minimum relay OFF
time had not yet elapsed.

### Manual ON

    Mode           : MANUAL
    Manual Request : ON
    Humidifier     : ON
    Timing Lock    : OFF

The relay activated correctly after the minimum OFF time elapsed.

### Manual OFF

    Mode           : MANUAL
    Manual Request : OFF
    Humidifier     : OFF

The explicit Manual OFF request stopped the relay immediately.

### Manual High-Humidity Safety

    Mode           : MANUAL
    Humidity       : 90.0 %
    Manual Request : ON
    Humidifier     : OFF
    RH Safety      : ACTIVE

The humidity safety system correctly overrode the Manual ON request.

### Manual Invalid Input

    Mode           : MANUAL
    Input VPD      : INVALID
    Humidity       : INVALID
    Humidifier     : OFF

The controller correctly entered fail-safe behavior.

### Return to Automatic Mode

    Mode           : AUTO
    Humidifier     : OFF
    Timing Lock    : ACTIVE

The transition back to Automatic mode safely forced the relay OFF and restarted
the minimum OFF timer.

### Automatic Control Restored

    Mode           : AUTO
    Input VPD      : 1.70 kPa
    VPD Status     : HIGH
    Humidifier     : ON

Automatic VPD control resumed correctly after the mode transition.

## Result

The AUTO / MANUAL operating-mode implementation operated correctly.

The firmware successfully validated:

- Automatic VPD operation
- Safe transition from AUTO to MANUAL
- Manual ON control
- Manual OFF control
- Manual relay timing protection
- Manual humidity safety override
- Manual invalid-input fail-safe
- Safe transition from MANUAL back to AUTO
- Restoration of automatic control

## Status

- [x] PASS
- [ ] FAIL

## Evidence

Serial Monitor screenshot:

    auto-manual-modes-pass.png

---

# SW-07 - Wi-Fi Provisioning

## Objective

Verify that the Smart Greenhouse Controller can be connected to a local
Wi-Fi network without storing the user's Wi-Fi credentials in the firmware.

The system must support:

- Initial Wi-Fi provisioning through a captive portal
- Persistent Wi-Fi credentials
- Automatic reconnection after restart
- Recovery when the configured Wi-Fi network is temporarily unavailable
- Offline operation when Wi-Fi is unavailable

## Test Results

### Initial Provisioning

With no usable Wi-Fi connection available, the controller created the
temporary configuration access point:

    SmartGreenhouse-Setup

The configuration portal was available at:

    192.168.4.1

The user successfully selected the local Wi-Fi network and entered its
credentials through the captive portal.

Result:

    Wi-Fi connection: PASS
    Wi-Fi Status    : CONNECTED

### Saved Credentials

After restarting the ESP8266, the controller automatically connected to
the previously configured Wi-Fi network without requiring the user to
enter the credentials again.

Result:

    Connecting to SAVED AP
    AutoConnect: SUCCESS
    Wi-Fi Status: CONNECTED

### Temporary Network Failure

The configured router was temporarily disabled.

After restarting, the controller attempted to connect to the saved network.

The connection failed because the router was unavailable, but the saved
credentials were preserved.

The controller then started the fallback configuration access point:

    SmartGreenhouse-Setup

This confirmed that the device remained recoverable without firmware
modification.

### Network Recovery

The router was restored without entering new Wi-Fi credentials.

After restarting the ESP8266, the controller successfully connected using
the previously saved credentials.

Result:

    Connecting to SAVED AP
    AutoConnect: SUCCESS
    Wi-Fi Status: CONNECTED

## Result

The Wi-Fi provisioning implementation operated correctly.

The firmware successfully validated:

- Captive portal provisioning
- User Wi-Fi configuration without source-code credentials
- Persistent Wi-Fi credentials
- Automatic reconnection after restart
- Preservation of credentials during temporary network failure
- Configuration portal fallback
- Successful reconnection after network restoration

## Status

- [x] PASS
- [ ] FAIL

## Evidence

Serial Monitor evidence:

    wifi-provisioning-pass.png

