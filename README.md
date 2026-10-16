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
- GY-BME/P280 environmental sensor
  - Air temperature
  - Relative humidity (if confirmed as BME280)
  - Atmospheric pressure
- Capacitive soil moisture sensor

### Actuator
- ACEBOTT 5V relay module
- SONGLE SRD-05VDC-SL-C relay

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

## Planned Measurements

- Air temperature
- Relative humidity
- Atmospheric pressure
- Soil moisture
- Vapor Pressure Deficit (VPD)

---

## Planned Automation

The first automated function will control a humidifier using the relay.

Future control functions may include:

- Irrigation
- Ventilation
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

Hardware test results and selected evidence will be stored in the
`validation` documentation.

---

## Current Status

### Hardware

- [x] WeMos D1 R2 selected
- [x] Prototype shield assembled
- [x] BME280 connection prepared
- [x] Soil moisture sensor connection prepared
- [x] Relay connection prepared

### Validation

- [ ] ESP8266 boot test
- [ ] I2C scan
- [ ] BME280/BMP280 identification
- [ ] Temperature reading
- [ ] Relative humidity reading
- [ ] Pressure reading
- [ ] Soil sensor raw ADC test
- [ ] Soil sensor calibration
- [ ] Relay ON/OFF test
- [ ] Combined hardware test
- [ ] VPD calculation

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

Copyright
Copyright © 2026 Arioner923. All rights reserved.
This repository is publicly available for viewing and educational
reference.
No permission is granted to copy, redistribute, modify, publish or use
the source code, documentation or hardware designs without explicit
permission from the author.
