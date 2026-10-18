# Smart Greenhouse Controller

An ESP8266-based smart greenhouse monitoring and automation project built as a
personal embedded / IoT project.

The current prototype monitors environmental and soil conditions, calculates
Air Vapor Pressure Deficit (VPD), and controls a humidifier through a protected
AUTO / MANUAL control layer. A local responsive web dashboard provides live
monitoring and control over Wi-Fi.

> 🚧 This project is under active development. The current hardware is a
> low-voltage prototype, not an industrial or mains-ready controller.

---

## Current Hardware

### Controller

- WeMos D1 R2 V2.1.0
- ESP8266

### Sensors

- BME280
  - Air temperature
  - Relative humidity
  - Atmospheric pressure
- Capacitive analog soil-moisture sensor

### Actuator Interface

- ACEBOTT 5V relay module
- SONGLE SRD-05VDC-SL-C relay
- Active-HIGH trigger verified

---

## Pin Assignment

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

All low-voltage modules share a common ground.

---

## Current Firmware Features

### Monitoring

- Air temperature
- Relative humidity
- Atmospheric pressure
- Raw soil ADC value
- Calibrated relative soil-moisture percentage
- Soil-moisture status classification
- Air VPD
- Wi-Fi connection information

### Control

- Automatic VPD-based humidifier control
- Manual humidifier control
- AUTO / MANUAL operating modes
- Minimum relay ON/OFF timing protection
- Maximum-humidity safety cutoff
- Invalid-input fail-safe
- Safe relay OFF state during startup and mode changes

### Connectivity and Web UI

- WiFiManager-based Wi-Fi provisioning
- Persistent Wi-Fi credentials
- Runtime Wi-Fi recovery
- Greenhouse operation independent of Wi-Fi availability
- Local mDNS hostname:

      http://smartgreenhouse.local/

- Responsive local web dashboard
- Live JSON status endpoint:

      GET /api/status

- Browser-side live updates without periodic full-page reloads
- Web control for AUTO / MANUAL and humidifier ON / OFF
- Server-side blocking of manual actuator commands while AUTO mode is active

---

## Soil Moisture Calibration

The current capacitive sensor uses the following relative calibration
references:

    Air reference   : 746
    Water reference : 390

Relative scale:

    0%   = Air reference
    100% = Water reference

Current provisional status zones:

| Relative Moisture | Status |
|---:|---|
| < 20% | VERY DRY |
| 20% to < 35% | DRY |
| 35% to < 60% | GOOD |
| 60% to < 80% | WET |
| >= 80% | VERY WET |

The percentage is a project-specific relative scale. It must not be interpreted
as volumetric water content (VWC), and the status boundaries may be refined
with additional measurements from the actual growing medium.

---

## VPD Profiles

The current firmware contains three configurable growth-stage profiles.
Runtime growth-stage selection is planned; at present the startup profile is
selected in `config.h`.

| Growth Stage | Min VPD | Target VPD | Max VPD |
|---|---:|---:|---:|
| Propagation / Early Veg | 0.4 kPa | 0.6 kPa | 0.8 kPa |
| Late Veg / Early Flower | 0.8 kPa | 1.0 kPa | 1.2 kPa |
| Mid / Late Flower | 1.2 kPa | 1.4 kPa | 1.6 kPa |

These are project configuration profiles rather than universal crop targets.

---

## Software Architecture

The firmware is intentionally split into small modules:

```text
smart_greenhouse_controller.ino
        |
        +-- sensors.*            sensor acquisition and validation
        +-- soil_moisture.*      relative moisture classification
        +-- vpd.*                Air VPD calculations
        +-- control.*            AUTO / MANUAL control and safety
        +-- wifi_provisioning.*  Wi-Fi provisioning, recovery and mDNS
        +-- web_server.*         HTTP routes, status API and web commands
        +-- dashboard_page.h     embedded HTML / CSS
        +-- dashboard_script.h   embedded browser-side JavaScript
        +-- config.h             hardware and operating configuration
        `-- types.h              shared data structures and enums
```

The web interface does not write the relay GPIO directly. Commands are passed
to the control module so relay timing, humidity safety and fail-safe behavior
remain authoritative.

See [`docs/software-architecture.md`](docs/software-architecture.md) for the
full architecture description.

---

## Validation Status

### Hardware

- [x] ESP8266 boot validation
- [x] I2C communication
- [x] BME280 identification (`0x60`, address `0x76`)
- [x] Temperature / humidity / pressure readings
- [x] Soil sensor raw ADC validation
- [x] Soil calibration references
- [x] Relay trigger logic
- [x] Combined hardware operation
- [x] Air VPD calculation

### Firmware

- [x] Modular sensor integration
- [x] VPD module integration
- [x] Automatic VPD control
- [x] Relay timing protection
- [x] Humidity safety cutoff
- [x] Invalid-input fail-safe
- [x] AUTO / MANUAL operation
- [x] Wi-Fi provisioning and runtime recovery
- [x] Offline greenhouse operation
- [x] Local web dashboard
- [x] Web-based controller operation
- [x] Local mDNS access
- [x] Soil-moisture status classification
- [x] Soil classification boundary validation

Validation notes and selected evidence are stored under `validation/`.

---

## Milestone Tags

- `v0.1-hardware-validation`
- `v0.2-automatic-vpd-control`

---

## Planned Development

Near-term development may include:

- Runtime growth-stage selection
- Persistent controller settings
- MLX90614 leaf-temperature sensing
- Leaf VPD
- TSL2591 light / lux monitoring
- Additional actuator channels

Longer-term ideas include:

- Historical data logging
- microSD storage
- Charts and event history
- Mobile application
- Optional remote access / cloud integration

These are roadmap ideas and are not part of the current validated prototype.

---

## Repository Structure

```text
smart-greenhouse-controller/
├── firmware/      main controller firmware
├── hardware/      standalone hardware-validation sketches
├── docs/          architecture and wiring documentation
├── validation/    test plans, results and evidence
├── assets/        project images and supporting assets
└── README.md
```

---

## Safety

The current prototype shield contains only low-voltage control electronics.

Any future 230V AC switching must be physically isolated from the prototype
shield and installed using appropriately rated components, enclosure,
connectors, strain relief and protection suitable for a humid environment.

---

## Copyright

Copyright © 2026 Arioner923. All rights reserved.

This repository is available for viewing and educational reference. No
permission is granted to copy, redistribute, modify, publish or use the source
code, documentation or hardware designs without explicit permission from the
author.
