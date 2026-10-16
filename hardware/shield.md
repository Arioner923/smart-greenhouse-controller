# Prototype Shield

This document describes the current prototype shield wiring for the Smart Greenhouse Controller.

## Controller

- WeMos D1 R2 V2.1.0
- ESP8266 based board

## Pin Mapping

| Function | WeMos D1 R2 Pin | ESP8266 GPIO |
|---|---|---|
| BME280 SCL | D1 | GPIO5 |
| BME280 SDA | D2 | GPIO4 |
| Soil Moisture Analog Output | A0 | ADC |
| Relay Signal | D5 | GPIO14 |

## Power Connections

| Device | Supply |
|---|---|
| BME280 | 3.3V |
| Soil Moisture Sensor | 3.3V |
| Relay Module | 5V |

All modules share a common GND.

## Wiring Summary

```text
D1 / GPIO5   -> BME280 SCL
D2 / GPIO4   -> BME280 SDA
A0           -> Soil Analog
D5 / GPIO14  -> Relay Signal

3.3V         -> BME280 + Soil Sensor
5V           -> Relay Module
GND          -> Common Ground
```

Notes
The shield has been designed so that sensors can be disconnected and replaced without desoldering the main controller board.
The relay control side operates at low voltage. Any future mains-voltage wiring must be kept physically isolated from the prototype shield.