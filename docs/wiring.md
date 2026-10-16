# Wiring Guide

This document describes the current wiring of the Smart Greenhouse Controller prototype.

## Main Controller

- WeMos D1 R2 V2.1.0
- ESP8266

---

## BME280 Environmental Sensor

The BME280 environmental sensor communicates with the ESP8266 through the I2C bus.

| BME280 Pin | WeMos D1 R2 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SCL | D1 / GPIO5 |
| SDA | D2 / GPIO4 |

Wiring:

    BME280              WeMos D1 R2
    --------------------------------
    VCC   ------------> 3.3V
    GND   ------------> GND
    SCL   ------------> D1 / GPIO5
    SDA   ------------> D2 / GPIO4

The breakout board is marked `GY-BME/P280`.

The exact sensor type will therefore be verified during the first hardware test to confirm whether the installed chip is a BME280 or BMP280.

A BME280 provides:

- Air temperature
- Relative humidity
- Atmospheric pressure

Relative humidity is required for the VPD calculation.

---

## Capacitive Soil Moisture Sensor

The capacitive soil moisture sensor provides an analog voltage representing the moisture level of the soil.

| Soil Sensor Connection | WeMos D1 R2 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| Analog Output | A0 |

Wiring:

    Soil Sensor          WeMos D1 R2
    --------------------------------
    VCC   ------------> 3.3V
    GND   ------------> GND
    AOUT  ------------> A0

The sensor is powered from 3.3V.

During the first tests, the firmware will display the raw ADC reading without converting it to a percentage.

A calibration procedure will later determine reference values for:

- Dry condition
- Wet condition

These values will then be used to calculate an estimated soil moisture percentage.

---

## 5V Relay Module

The current relay module is an ACEBOTT 5V relay module equipped with a SONGLE SRD-05VDC-SL-C relay.

The low-voltage connector contains:

- S = Signal
- V = 5V supply
- G = Ground

| Relay Connection | WeMos D1 R2 |
|---|---|
| V / VCC | 5V |
| G / GND | GND |
| S / Signal | D5 / GPIO14 |

Wiring:

    Relay Module         WeMos D1 R2
    --------------------------------
    V     ------------> 5V
    G     ------------> GND
    S     ------------> D5 / GPIO14

The relay trigger logic has not yet been verified.

The first relay test will determine whether the module is:

- Active HIGH
or
- Active LOW

No mains-powered device will be connected during the initial relay test.

---

## Power Distribution

The prototype shield uses three common power rails:

    3.3V BUS
    5V BUS
    GND BUS

### 3.3V Rail

Currently supplies:

- BME280
- Capacitive soil moisture sensor

### 5V Rail

Currently supplies:

- ACEBOTT relay module

### Ground Rail

All low-voltage devices share a common ground:

- WeMos D1 R2
- BME280
- Soil moisture sensor
- Relay module

---

## Current Pin Assignment

| Function | WeMos D1 R2 Pin | ESP8266 Pin |
|---|---|---|
| BME280 SCL | D1 | GPIO5 |
| BME280 SDA | D2 | GPIO4 |
| Soil Moisture Analog | A0 | ADC |
| Relay Signal | D5 | GPIO14 |

---

## Current Hardware Overview

    WeMos D1 R2 / ESP8266
             |
             |--- D1 / GPIO5 ---> BME280 SCL
             |
             |--- D2 / GPIO4 ---> BME280 SDA
             |
             |--- A0 -----------> Soil Sensor Analog
             |
             |--- D5 / GPIO14 --> Relay Signal
             |
             |--- 3.3V ---------> BME280
             |                 --> Soil Sensor
             |
             |--- 5V -----------> Relay Module
             |
             `--- GND ----------> Common Ground

---

## Current Shield Status

- [x] WeMos D1 R2 installed
- [x] 3.3V power distribution prepared
- [x] 5V power distribution prepared
- [x] Common GND prepared
- [x] BME280 connector prepared
- [x] Soil moisture sensor connector prepared
- [x] Relay connector prepared

---

## Pending Hardware Validation

- [ ] Verify ESP8266 boot
- [ ] Scan I2C bus
- [ ] Detect BME280/BMP280
- [ ] Read air temperature
- [ ] Read relative humidity
- [ ] Read atmospheric pressure
- [ ] Read raw soil ADC value
- [ ] Calibrate soil moisture sensor
- [ ] Determine relay trigger logic
- [ ] Test relay ON/OFF
- [ ] Test all devices together

---

## Safety

The prototype shield contains only low-voltage control electronics.

The 230V AC side of the relay must not be routed through the prototype shield tracks.

Any future mains-voltage wiring must be physically separated from the low-voltage electronics and installed inside a suitable insulated enclosure.

Special care is required because the final system is intended for use in an environment where water and high humidity may be present.