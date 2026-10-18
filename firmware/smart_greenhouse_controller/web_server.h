#pragma once

#include "types.h"

/*
 * Smart Greenhouse Controller
 * Local Web Dashboard and Control Interface
 *
 * Provides the local monitoring dashboard, the live JSON status API and
 * web-based operating-mode / humidifier commands. Actuator commands are
 * forwarded to the control module so existing safety protections remain
 * authoritative.
 */


/*
 * Configure the web server routes.
 *
 * If Wi-Fi is already connected, the server
 * starts immediately.
 *
 * If Wi-Fi is unavailable, the server will
 * start automatically when a connection
 * becomes available.
 */
void initializeWebServer();


/*
 * Update the latest greenhouse data displayed
 * by the dashboard.
 */
void updateWebServerData(
    const SensorData& sensorData,
    float airVPD,
    const ControlState& controlState
);


/*
 * Process incoming HTTP requests.
 *
 * Must be called repeatedly from loop().
 */
void processWebServer();