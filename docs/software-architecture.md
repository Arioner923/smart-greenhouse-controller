# Software Architecture

This document describes the software architecture of the Smart Greenhouse
Controller firmware.

The purpose of this document is to define the system responsibilities,
data flow, software modules and module interfaces before implementation.

The architecture is intentionally kept modular but simple enough for an
ESP8266-based embedded project.

---

# 1. Current Firmware Scope

The first production firmware version will support the currently validated
hardware:

- WeMos D1 R2 V2.1.0 / ESP8266
- BME280 environmental sensor
- Capacitive soil moisture sensor
- ACEBOTT 5V relay module

The first automation feature will control a humidifier using Air VPD.

The following features are outside the scope of the first firmware version:

- Leaf temperature measurement
- Leaf VPD
- CO2 measurement
- Light measurement
- PPFD / DLI calculation
- Irrigation automation
- Fan control
- Web dashboard
- Wi-Fi communication
- Data logging

These features may be added in future development stages.

---

# 2. System Requirements

The firmware must:

1. Initialize all currently connected hardware.
2. Read air temperature from the BME280.
3. Read relative humidity from the BME280.
4. Read atmospheric pressure from the BME280.
5. Read the raw analog value from the soil moisture sensor.
6. Convert the soil sensor RAW value to a relative moisture percentage.
7. Calculate Air Vapor Pressure Deficit (VPD).
8. Use a selected growth-stage VPD profile.
9. Determine whether the measured VPD is:
   - LOW
   - IN RANGE
   - HIGH
10. Automatically control the humidifier relay when VPD is too high.
11. Prevent unnecessary rapid relay switching.
12. Keep the relay OFF if sensor readings are invalid.
13. Initialize the relay in the safe OFF state.
14. Print system measurements and control status to the Serial Monitor.

---

# 3. Inputs and Outputs

## Inputs

### BME280

Provides:

- Air temperature
- Relative humidity
- Atmospheric pressure

Communication:

    I2C

Connections:

    SCL -> D1 / GPIO5
    SDA -> D2 / GPIO4

I2C address:

    0x76

---

### Soil Moisture Sensor

Provides:

- Analog soil moisture signal

Connection:

    Analog Output -> A0

Calibration references:

    Air reference   : 746
    Water reference : 390

The calculated percentage represents a relative moisture scale.

It does not represent absolute volumetric water content.

---

## Outputs

### Relay Module

Purpose:

    Humidifier control

Connection:

    Relay Signal -> D5 / GPIO14

Verified trigger logic:

    LOW  -> Relay OFF
    HIGH -> Relay ON

The relay must always initialize in the OFF state.

---

# 4. Data Flow

The firmware data flow is:

```text
BME280
   │
   ├── Temperature
   ├── Relative Humidity
   └── Pressure
          │
          │
          ├──────────────┐
          │              │
Temperature + RH         │
       │                 │
       ▼                 │
   Air VPD               │
       │                 │
       ▼                 │
Growth Stage Profile     │
       │                 │
       ▼                 │
  Control Logic          │
       │                 │
       ▼                 │
      Relay              │
       │                 │
       ▼                 │
   Humidifier            │
                         │
Soil Sensor              │
     │                   │
     ▼                   │
  RAW ADC                │
     │                   │
     ▼                   │
Relative Soil %          │
                         │
                         ▼
                   Serial Output
```

---

# 5. Main Software Responsibilities

The firmware is divided into the following responsibilities:

```text
Main Application
│
├── Configuration
├── Shared Data Types
├── Sensor Management
├── VPD Calculations
└── Control Logic
```

Each responsibility is implemented as a separate software module where
appropriate.

---

# 6. Module Architecture

## 6.1 Main Application

Files:

    smart_greenhouse_controller.ino

Responsibilities:

- Run system initialization.
- Execute the main program loop.
- Coordinate the software modules.
- Avoid containing detailed sensor or control implementation.

The main file should describe the high-level operation of the system.

Conceptually:

```text
setup
  ↓
Initialize Sensors
  ↓
Initialize Control System
  ↓
Initialize Serial Output


loop
  ↓
Read Sensors
  ↓
Calculate VPD
  ↓
Evaluate Control Logic
  ↓
Update Relay
  ↓
Print Status
  ↓
Repeat
```

---

## 6.2 Configuration Module

File:

    config.h

Responsibilities:

- Hardware pin definitions
- I2C addresses
- Soil calibration references
- Serial baud rate
- Selected growth stage
- Control configuration

Examples of configuration values:

```text
Relay Pin
BME280 Address
Soil Air Reference
Soil Water Reference
Serial Baud Rate
Selected Growth Stage
```

Configuration values should not be duplicated throughout the source code.

This provides a single source of truth for system configuration.

---

# 7. Shared Data Types

File:

    types.h

This file contains data structures and enumerations shared between multiple
software modules.

---

## Growth Stage

The system will initially support three selectable VPD profiles:

```text
Propagation / Early Veg
Late Veg / Early Flower
Mid / Late Flower
```

Conceptual type:

```cpp
enum class GrowthStage
{
    PropagationEarlyVeg,
    LateVegEarlyFlower,
    MidLateFlower
};
```

---

## VPD Profile

Each growth stage contains:

- Minimum VPD
- Target VPD
- Maximum VPD

Conceptual structure:

```cpp
struct VPDProfile
{
    const char* name;
    float minVPD;
    float targetVPD;
    float maxVPD;
};
```

Initial profiles:

| Growth Stage | Minimum | Target | Maximum |
|---|---:|---:|---:|
| Propagation / Early Veg | 0.4 kPa | 0.6 kPa | 0.8 kPa |
| Late Veg / Early Flower | 0.8 kPa | 1.0 kPa | 1.2 kPa |
| Mid / Late Flower | 1.2 kPa | 1.4 kPa | 1.6 kPa |

These values are treated as configurable project profiles and may be adjusted
during later greenhouse testing.

---

## Sensor Measurements

A shared sensor data structure may contain:

```cpp
struct SensorData
{
    float temperature;
    float humidity;
    float pressure;

    int soilRaw;
    float soilPercent;

    bool environmentalSensorValid;
};
```

This allows the sensor module to provide measurements to the rest of the
application in a structured form.

---

## VPD Status

The controller must classify VPD into one of three states:

```cpp
enum class VPDStatus
{
    Low,
    InRange,
    High
};
```

---

# 8. Sensor Module

Files:

    sensors.h
    sensors.cpp

Responsibilities:

- Initialize the BME280.
- Read air temperature.
- Read relative humidity.
- Read atmospheric pressure.
- Read soil moisture RAW ADC.
- Convert soil RAW value to relative percentage.
- Detect invalid environmental sensor readings.

The sensor module should only be responsible for obtaining and preparing
sensor measurements.

It should not decide whether the humidifier should turn ON or OFF.

---

## Sensor Module Interface

The public interface may provide functions conceptually similar to:

```cpp
bool initializeSensors();

SensorData readSensors();
```

The exact implementation will be defined during firmware development.

---

# 9. VPD Calculation Module

Files:

    vpd.h
    vpd.cpp

Responsibilities:

- Calculate saturation vapor pressure.
- Calculate Air VPD.
- Provide reusable VPD calculation functions.
- Support future Leaf VPD calculation.

The VPD module should contain mathematical calculations only.

It should not directly access the relay or control hardware.

---

## Air VPD

Air VPD is calculated from:

- Air temperature
- Relative humidity

Conceptual calculation:

```text
SVP(T) = 0.6108 × exp((17.27 × T) / (T + 237.3))

Air VPD = SVP(T) × (1 - RH / 100)
```

Result:

    kPa

---

## Future Leaf VPD

When an MLX90614 infrared leaf temperature sensor is added, Leaf VPD may be
calculated using:

```text
Leaf VPD =
SVP(Tleaf) - [SVP(Tair) × RH / 100]
```

Leaf VPD is not part of the first firmware implementation.

---

# 10. Control Module

Files:

    control.h
    control.cpp

Responsibilities:

- Evaluate current VPD.
- Compare VPD against the selected profile.
- Determine VPD status.
- Control the humidifier relay.
- Prevent rapid relay switching.
- Enforce safe relay behavior.
- Keep the relay OFF if required sensor data is invalid.

The control module should not directly read the BME280.

It receives already prepared measurements and calculated VPD values.

---

# 11. Automatic VPD Control Strategy

The current hardware can actively reduce VPD using a humidifier.

Increasing humidity causes:

```text
Relative Humidity ↑
        ↓
VPD ↓
```

Therefore, the current system provides one-direction environmental control.

It can actively correct HIGH VPD.

It cannot actively correct LOW VPD until ventilation, dehumidification or
another suitable actuator is added.

---

## Control States

### VPD HIGH

Condition:

```text
Current VPD > Profile Maximum
```

Action:

```text
Humidifier ON
```

---

### VPD IN RANGE

Condition:

```text
Profile Minimum <= Current VPD <= Profile Maximum
```

Action depends on the previous relay state and hysteresis strategy.

If the humidifier was activated because of HIGH VPD, it may remain ON until
VPD reaches the configured target.

---

### VPD LOW

Condition:

```text
Current VPD < Profile Minimum
```

Action:

```text
Humidifier OFF
```

The current hardware cannot actively increase VPD.

---

# 12. Relay Hysteresis

The relay should not switch repeatedly when the measured VPD is close to a
single threshold.

The first control strategy will use the growth-stage target as the relay OFF
point.

Example:

```text
Growth Stage:
Mid / Late Flower

Minimum : 1.20 kPa
Target  : 1.40 kPa
Maximum : 1.60 kPa
```

Control behavior:

```text
VPD > 1.60
    → Humidifier ON

Humidifier already ON
and
VPD > 1.40
    → Keep Humidifier ON

VPD <= 1.40
    → Humidifier OFF

VPD < 1.20
    → Humidifier OFF
    → VPD Status LOW
```

This creates a control band between the ON and OFF thresholds and reduces
rapid relay switching.

The hysteresis strategy may be refined after real greenhouse testing.

---

# 13. Safety Logic

The first firmware version must include the following safe behavior:

### Startup

The relay must initialize as:

```text
OFF
```

before sensor initialization or control decisions occur.

### Invalid Sensor Data

If valid temperature or humidity data is unavailable:

```text
Humidifier OFF
```

Automatic VPD control must not continue using invalid environmental data.

### Reset

After an ESP8266 reset, the relay should return to the safe OFF state before
normal control operation resumes.

### Mains Voltage

The ESP8266 firmware only controls the low-voltage relay input.

230V AC wiring must remain physically isolated from the prototype shield and
must not be part of firmware validation tests.

---

# 14. Serial Monitor Output

The firmware should provide a readable diagnostic output.

Example:

```text
================================
 Smart Greenhouse Controller
================================

Growth Stage : Mid / Late Flower

Temperature  : 25.60 C
Humidity     : 45.99 %
Pressure     : 1006.73 hPa

Soil RAW     : 473
Soil Moist.  : 76.7 %

Air VPD      : 1.77 kPa
Target       : 1.40 kPa
Range        : 1.20 - 1.60 kPa
VPD Status   : HIGH

Humidifier   : ON
```

The Serial Monitor will be used during development and validation before a
future web dashboard is implemented.

---

# 15. Firmware File Structure

The first modular firmware version will use:

```text
firmware/
└── smart_greenhouse_controller/
    ├── smart_greenhouse_controller.ino
    ├── config.h
    ├── types.h
    ├── sensors.h
    ├── sensors.cpp
    ├── vpd.h
    ├── vpd.cpp
    ├── control.h
    └── control.cpp
```

---

# 16. File Responsibilities

| File | Responsibility |
|---|---|
| `smart_greenhouse_controller.ino` | Application startup, main loop and module coordination |
| `config.h` | Hardware pins, addresses, calibration and configuration |
| `types.h` | Shared enums and data structures |
| `sensors.h` | Public sensor module interface |
| `sensors.cpp` | Sensor initialization, acquisition and soil conversion |
| `vpd.h` | Public VPD calculation interface |
| `vpd.cpp` | Air VPD mathematical implementation |
| `control.h` | Public control module interface |
| `control.cpp` | VPD state evaluation and relay control |

---

# 17. Design Principles

The firmware architecture follows the following principles.

## Separation of Concerns

Sensor acquisition, calculations and control decisions are kept separate.

## High Cohesion

Functions with related responsibilities remain inside the same module.

## Low Coupling

Changes to one module should require minimal changes to other modules.

Example:

Changing the BME280 implementation should primarily affect:

```text
sensors.cpp
```

Changing the VPD calculation should primarily affect:

```text
vpd.cpp
```

Changing the humidifier control strategy should primarily affect:

```text
control.cpp
```

## Single Source of Truth

Hardware configuration and calibration values should be defined once and reused
throughout the application.

## Incremental Development

New modules and automation features should only be added after the current
implementation has been tested and validated.

---

# 18. Validation Plan

The first modular firmware will be validated through:

## Test 10 - Automatic VPD Control

The test will verify:

- Correct sensor acquisition
- Correct Air VPD calculation
- Correct growth-stage profile selection
- Correct VPD status classification
- Correct relay activation
- Correct relay deactivation
- Hysteresis behavior
- Safe relay startup
- Safe response to invalid environmental readings

No 230V AC load will be connected during the initial control validation.

Relay operation will initially be verified using:

- Relay indicator LED
- Mechanical relay click
- Serial Monitor output

---

# 19. Future Architecture Expansion

Future hardware may introduce additional modules.

Examples:

```text
Leaf Temperature
        ↓
      Leaf VPD

Light Sensor
        ↓
Lux / Estimated PPFD
        ↓
       DLI

CO2 Sensor
        ↓
CO2 Monitoring

Wi-Fi
        ↓
Web Dashboard / Data Logging
```

The current architecture is designed so these features can be added without
rewriting the entire application.

---

# Architecture Status

Current architecture version:

    v1

Status:

    DESIGN READY FOR IMPLEMENTATION

Previous milestone:

    v0.1-hardware-validation

Next development step:

    Implement modular firmware and perform
    Test 10 - Automatic VPD Control