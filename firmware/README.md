# Firmware

ESP8266 firmware for the Smart Greenhouse Controller.

The main sketch is located in `smart_greenhouse_controller/`. The firmware is
split into dedicated modules for sensors, VPD, soil classification, control,
Wi-Fi provisioning and the local web interface.

The dashboard HTML/CSS and JavaScript are embedded in firmware through
`dashboard_page.h` and `dashboard_script.h`; no LittleFS upload is currently
required.
