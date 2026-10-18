# Software Architecture

This document describes the current software architecture of the Smart
Greenhouse Controller firmware.

The project uses a modular but deliberately lightweight design suitable for an
ESP8266 prototype. Hardware access, control decisions, networking and the web
interface are kept separate so that one subsystem can evolve without forcing
unrelated changes elsewhere.

---

## 1. Current Scope

Current hardware:

- WeMos D1 R2 V2.1.0 / ESP8266
- BME280 environmental sensor
- Capacitive analog soil-moisture sensor
- ACEBOTT 5V relay module

Current firmware capabilities:

- Environmental sensing
- Relative soil-moisture measurement and classification
- Air VPD calculation
- Configurable growth-stage VPD profiles
- Automatic VPD-based humidifier control
- Manual humidifier control
- Relay timing protection
- Relative-humidity safety cutoff
- Invalid-input fail-safe behavior
- Wi-Fi provisioning and recovery
- Local mDNS hostname
- Local web dashboard and web-based control
- JSON status API
- Browser-side live dashboard updates

Future features such as additional sensors, data logging, mobile applications,
cloud connectivity and remote access are outside the current implementation.

---

## 2. High-Level Data Flow

```text
BME280 ---------+
                |
Soil Sensor ----+--> Sensor Module --> SensorData
                                      |
                                      +--> Soil Classification
                                      |
                                      +--> Air VPD Calculation
                                                |
                                                v
                                         Control Module
                                                |
                                                v
                                             Relay

SensorData + VPD + ControlState
              |
              +--> Serial diagnostics
              |
              +--> Local Web Server
                        |
                        +--> GET /api/status
                        +--> POST control commands
                        +--> Dashboard HTML / CSS
                        `--> Dashboard JavaScript
```

The control loop does not depend on Wi-Fi. Network loss disables remote/local
browser access but does not stop sensor acquisition or greenhouse control.

---

## 3. Main Application

File:

    smart_greenhouse_controller.ino

Responsibilities:

- Initialize control hardware in a safe OFF state.
- Initialize sensors.
- Initialize Wi-Fi provisioning and the local web server.
- Run the non-blocking Wi-Fi and HTTP service functions.
- Execute the periodic sensor / VPD / control cycle.
- Publish the latest control-cycle snapshot to the web layer.
- Print diagnostics to the Serial Monitor.

The main file coordinates modules but does not implement low-level sensor,
network or relay-control rules itself.

---

## 4. Configuration and Shared Types

### `config.h`

Central source of configuration for:

- Serial baud rate
- BME280 I2C address
- Soil calibration references
- Soil status boundaries
- Relay GPIO and active level
- Relay minimum ON/OFF timing
- Humidity safety ceiling
- Startup control mode
- Growth-stage VPD profiles
- Sensor read interval
- Wi-Fi setup-portal settings
- mDNS hostname

### `types.h`

Contains shared data structures and enums:

- `GrowthStage`
- `VPDProfile`
- `SoilMoistureStatus`
- `SensorData`
- `VPDStatus`
- `ControlMode`
- `ControlState`

These types form the interface between otherwise independent modules.

---

## 5. Sensor Module

Files:

    sensors.h
    sensors.cpp

Responsibilities:

- Initialize the ESP8266 I2C bus.
- Initialize the BME280 at the validated address `0x76`.
- Read temperature, humidity and pressure.
- Read the analog soil sensor through `A0`.
- Convert soil RAW data to the calibrated relative percentage.
- Classify the soil value through the soil-moisture module.
- Mark invalid environmental measurements.

The sensor module does not decide whether an actuator should run.

---

## 6. Soil Moisture Classification

Files:

    soil_moisture.h
    soil_moisture.cpp

Responsibilities:

- Convert a relative soil percentage into one of:
  - `VERY DRY`
  - `DRY`
  - `GOOD`
  - `WET`
  - `VERY WET`
- Provide readable status text for diagnostics and the dashboard.

The boundaries are stored in `config.h` so the classification policy can be
changed without duplicating threshold values in other modules.

The percentage is a project-specific relative scale and is not volumetric
water content (VWC).

---

## 7. VPD Module

Files:

    vpd.h
    vpd.cpp

Responsibilities:

- Calculate saturation vapor pressure from air temperature.
- Calculate Air Vapor Pressure Deficit from temperature and relative humidity.
- Reject invalid humidity inputs.

Conceptually:

```text
SVP(T) = 0.6108 * exp((17.27 * T) / (T + 237.3))

Air VPD = SVP(Tair) * (1 - RH / 100)
```

Leaf VPD is not implemented yet.

---

## 8. Control Module

Files:

    control.h
    control.cpp

Responsibilities:

- Maintain AUTO / MANUAL operating mode.
- Control the humidifier relay.
- Apply VPD hysteresis behavior in Automatic mode.
- Apply manual ON/OFF requests in Manual mode.
- Enforce minimum relay ON/OFF timing.
- Enforce the maximum-humidity safety cutoff.
- Force the relay OFF when control inputs are invalid.
- Force a safe OFF transition when operating mode changes.

### Control Priority

Safety decisions take priority over normal control requests:

```text
Invalid input
    -> relay OFF

Humidity safety limit reached
    -> relay OFF

Otherwise
    -> AUTO or MANUAL logic
```

The web server never writes the relay GPIO directly. Web commands update the
control module, which remains the authoritative actuator layer.

---

## 9. Wi-Fi Provisioning and mDNS

Files:

    wifi_provisioning.h
    wifi_provisioning.cpp

Responsibilities:

- Connect using saved Wi-Fi credentials when available.
- Start a WiFiManager configuration portal when required.
- Keep the setup portal non-blocking so control logic continues operating.
- Recover from temporary network loss.
- Start and maintain the mDNS hostname:

      http://smartgreenhouse.local/

Wi-Fi is treated as an optional interface to the controller rather than a
requirement for greenhouse operation.

---

## 10. Local Web Server

Files:

    web_server.h
    web_server.cpp

Responsibilities:

- Serve the local dashboard.
- Serve the embedded dashboard JavaScript.
- Expose the live JSON status API.
- Accept operating-mode commands.
- Accept manual humidifier commands only when Manual mode is active.
- Forward all control requests to the control module.

### Current Routes

```text
GET  /                         dashboard
GET  /dashboard.js             browser-side live-update logic
GET  /api/status               current controller snapshot as JSON

POST /control/mode/auto        select AUTO mode
POST /control/mode/manual      select MANUAL mode
POST /control/humidifier/on    request humidifier ON
POST /control/humidifier/off   request humidifier OFF
```

Manual humidifier routes also enforce Manual mode on the server side. Disabled
buttons in the UI are therefore only the first layer of protection.

---

## 11. Dashboard Resources

### `dashboard_page.h`

Contains the static HTML shell and CSS styling in `PROGMEM`.

### `dashboard_script.h`

Contains browser-side JavaScript in `PROGMEM`.

The JavaScript requests:

    GET /api/status

approximately every two seconds and updates existing DOM elements without
periodically reloading the entire page.

This separation keeps web UI code out of `web_server.cpp` while avoiding the
additional deployment complexity of LittleFS during the current prototype
stage.

---

## 12. Status API

`GET /api/status` returns the latest controller snapshot as JSON, including:

- Temperature
- Relative humidity
- Atmospheric pressure
- Soil moisture percentage
- Soil status
- Air VPD
- VPD target and range
- VPD status
- Operating mode
- Growth stage
- Actual humidifier state
- Control validity
- Relay timing lock
- Humidity safety state
- Wi-Fi state
- Local IP address
- RSSI
- Age of the latest sensor update

This API also provides a clean foundation for future clients such as a mobile
application without requiring those clients to parse the HTML dashboard.

---

## 13. Current Limitations

The current prototype intentionally does not yet include:

- Runtime growth-stage selection
- Persistent controller settings beyond Wi-Fi credentials
- User authentication for the local dashboard
- TLS / HTTPS on the ESP8266 web interface
- Historical sensor storage
- microSD logging
- Cloud synchronization
- Mobile application
- Remote Internet control

These are possible future development stages, not requirements of the current
local prototype.

---

## 14. Design Principle

The central design rule is:

```text
Sensors / Control / Safety
          |
          v
       Local API
       /       \
      v         v
 Web Dashboard  Future Clients
```

The controller must continue operating safely even when every user-interface
or network component is unavailable.
