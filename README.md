# Smart Greenhouse Controller

An ESP8266-based smart greenhouse monitoring and automation project.

The system is designed to monitor environmental and soil conditions and,
at later development stages, automatically control greenhouse equipment
such as a humidifier, ventilation fan and irrigation system.

> 🚧 This project is currently under active development.

---

## Current Hardware

### Main Controller

- WeMos D1 R2 V2.1.0
- ESP8266-based development board

### Sensors

- BME280 environmental sensor
  - Air temperature
  - Relative humidity
  - Atmospheric pressure
- Capacitive soil moisture sensor

### Actuator

- ACEBOTT 5V relay module
- SONGLE SRD-05VDC-SL-C relay
- Active-HIGH trigger verified

---

## Current Shield Pinout

| Function | WeMos D1 R2 Pin | ESP8266 GPIO |
|---|---|---|
| BME280 SCL | D1 | GPIO5 |
| BME280 SDA | D2 | GPIO4 |
| Soil Sensor Analog | A0 | ADC |
| Relay Signal | D5 | GPIO14 |

### Power

| Device | Supply |
|---|---|
| BME280 | 3.3V |
| Soil Moisture Sensor | 3.3V |
| Relay Module | 5V |

All low-voltage modules share a common GND.

---

## Current Measurements

The current prototype can measure and calculate:

- Air temperature
- Relative humidity
- Atmospheric pressure
- Raw soil moisture ADC value
- Estimated soil moisture percentage
- Air Vapor Pressure Deficit (VPD)

---

## Soil Moisture Calibration

The current capacitive soil moisture sensor was calibrated using:

    Air reference   : 746
    Water reference : 390

Relative scale:

    0%   = Air reference
    100% = Water reference

Real soil validation:

    Dry soil                : ~744 RAW
    Well-watered soil       : 469 - 481 RAW

The calculated percentage represents a relative moisture scale and does not
represent absolute volumetric water content.

---

## Current Automation Hardware

The relay module has been validated successfully.

Observed behavior:

    LOW  -> Relay OFF
    HIGH -> Relay ON

The relay will initially be used to control a humidifier.

---

## VPD

Air VPD is calculated using:

- BME280 air temperature
- BME280 relative humidity

A future MLX90614 infrared temperature sensor may be added to measure leaf
temperature and calculate Leaf VPD.

---

## VPD Control Profiles

The controller is designed to support selectable growth-stage VPD profiles.

### Propagation / Early Veg

    VPD Range  : 0.4 - 0.8 kPa
    Target VPD : 0.6 kPa

### Late Veg / Early Flower

    VPD Range  : 0.8 - 1.2 kPa
    Target VPD : 1.0 kPa

### Mid / Late Flower

    VPD Range  : 1.2 - 1.6 kPa
    Target VPD : 1.4 kPa

Automatic humidifier control will use hysteresis to prevent rapid relay
switching.

---

## Planned Automation

The first automatic control function will regulate VPD using the humidifier.

Future control functions may include:

- Irrigation
- Ventilation
- Dehumidification
- Lighting
- Environmental alarms

---

## Planned Sensors and Upgrades

Future development may include:

- MLX90614 infrared leaf temperature sensor
- Light / lux sensor
- Estimated PPFD
- Daily Light Integral (DLI)
- CO₂ sensor
- Multiple soil moisture sensors
- External ADC
- Water level sensor
- Web dashboard
- Data logging

---

## Development Method

The project is developed incrementally.

Each hardware and firmware feature is:

1. Implemented
2. Tested on real hardware
3. Validated
4. Documented
5. Committed separately to the repository

Hardware and firmware validation results, together with selected evidence,
are stored in the `validation` documentation.

---

## Current Project Status

### Hardware

- [x] WeMos D1 R2 selected
- [x] Prototype shield assembled
- [x] BME280 connected
- [x] Soil moisture sensor connected
- [x] Relay module connected

### Hardware Validation

- [x] ESP8266 boot validation
- [x] I2C communication
- [x] BME280 identification
- [x] Temperature measurement
- [x] Relative humidity measurement
- [x] Atmospheric pressure measurement
- [x] Soil sensor raw ADC validation
- [x] Soil sensor calibration
- [x] Relay ON/OFF validation
- [x] Combined hardware test
- [x] Air VPD calculation

### Firmware Development

- [x] Software architecture defined
- [x] Shared data types implemented
- [x] System configuration implemented
- [x] Sensor module implemented
- [x] Sensor module integration validated
- [x] VPD calculation module implemented
- [x] VPD module integration validated
- [x] Control module implemented
- [x] Automatic VPD control validated

### Hardware Validation

- [x] ESP8266 boot validation
- [x] I2C communication
- [x] BME280 identification
- [x] Temperature measurement
- [x] Relative humidity measurement
- [x] Atmospheric pressure measurement
- [x] Soil sensor raw ADC validation
- [x] Soil sensor calibration
- [x] Relay ON/OFF validation
- [x] Combined hardware test
- [x] Air VPD calculation

### Completed Milestone

Initial hardware validation completed successfully.

Git milestone tag:

    v0.1-hardware-validation

### Current Development Stage

The project is currently in the modular firmware implementation phase.

Completed:

- Software architecture design
- Shared firmware data types
- Centralized system configuration
- Modular sensor implementation
- Sensor module integration test

Current next steps:

- Implement the control module
- Add growth-stage profile selection
- Implement automatic humidifier control
- Validate relay hysteresis and safety behavior

---

## Repository Structure

```text
smart-greenhouse-controller/
├── README.md
├── firmware/
├── hardware/
├── docs/
├── validation/
└── assets/
```

## Safety
The prototype shield contains only low-voltage control electronics.
Any future 230V AC wiring must remain physically isolated from the prototype
shield and must be installed inside a suitable insulated enclosure.
Special care is required because the final system is intended for operation in
an environment where water and high humidity may be present.
---

## Copyright
Copyright © 2026 Arioner923. All rights reserved.
This repository is publicly available for viewing and educational reference.
No permission is granted to copy, redistribute, modify, publish or use the
source code, documentation or hardware designs without explicit permission
from the author.