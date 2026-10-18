#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <math.h>

#include "types.h"
#include "control.h"
#include "web_server.h"


/*
 * Smart Greenhouse Controller
 * Local Web Dashboard + Web Control
 *
 * The page is sent in small HTTP chunks instead of being assembled as one
 * large String. This avoids large contiguous RAM allocations on ESP8266.
 */


/*
 * Local HTTP server.
 *
 * Port 80 allows access using:
 *
 * http://DEVICE_IP/
 */
static ESP8266WebServer server(80);


/*
 * Latest greenhouse measurements.
 */
static SensorData latestSensorData;
static ControlState latestControlState;

static float latestAirVPD = NAN;

static bool dashboardDataAvailable = false;
static bool webServerStarted = false;
static bool routesConfigured = false;

static unsigned long lastDataUpdateTime = 0;


/*
 * Send a small HTML fragment and yield to the ESP8266 background tasks.
 *
 * This is intentionally used instead of building one very large String.
 */
static void sendHtml(const __FlashStringHelper* content)
{
    server.sendContent(String(content));
    yield();
}


static void sendHtml(const String& content)
{
    server.sendContent(content);
    yield();
}


/*
 * Start the local HTTP server when Wi-Fi is available.
 */
static void startWebServerIfPossible()
{
    if (webServerStarted)
    {
        return;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        return;
    }

    server.begin();

    webServerStarted = true;

    Serial.println();
    Serial.println("================================");
    Serial.println(" Local Web Dashboard");
    Serial.println("================================");

    Serial.println("Web dashboard : READY");

    Serial.print("Dashboard URL : http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");

    Serial.println();
}


/*
 * Generate the main dashboard page.
 *
 * IMPORTANT:
 * The response uses chunked transfer encoding so the ESP8266 does not need
 * to keep the entire HTML page in one large String allocation.
 */
static void handleDashboard()
{
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");

    sendHtml(F(
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<meta http-equiv='refresh' content='3'>"
        "<title>Smart Greenhouse</title>"
        
        "<style>"
        "body{"
        "margin:0;"
        "font-family:Arial,sans-serif;"
        "background:#f4f7f5;"
        "color:#1f2933;"
        "}"

        ".container{"
        "max-width:900px;"
        "margin:auto;"
        "padding:20px;"
        "}"

        "header{"
        "background:#ffffff;"
        "padding:22px;"
        "border-radius:14px;"
        "margin-bottom:18px;"
        "box-shadow:0 2px 10px rgba(0,0,0,.08);"

        "}"
        "h1{"
        "margin:0 0 6px 0;"
        "font-size:28px;"
        "}"
        
        ".subtitle{"
        "color:#667085;"
        "}"

        ".grid{"
        "display:grid;"
        "grid-template-columns:repeat(auto-fit,minmax(190px,1fr));"
        "gap:14px;"
        "margin-bottom:18px;"
        "}"

        ".card{"
        "background:#ffffff;"
        "padding:18px;"
        "border-radius:14px;"
        "box-shadow:0 2px 10px rgba(0,0,0,.07);"
        "}"

        ".label{"
        "font-size:13px;"
        "color:#667085;"
        "margin-bottom:8px;"
        "}"
        
        ".value{"
        "font-size:25px;"
        "font-weight:bold;"
        "}"

        ".section-title{"
        "font-size:18px;"
        "font-weight:bold;"
        "margin:22px 0 10px;"
        "}"

        ".status{"
        "font-weight:bold;"
        "}"

        ".footer{"
        "text-align:center;"
        "color:#667085;"
        "font-size:12px;"
        "padding:18px;"
        "}"

        ".controls{"
        "display:flex;"
        "gap:10px;"
        "flex-wrap:wrap;"
        "}"

        ".btn{"
        "border:0;"
        "border-radius:10px;"
        "padding:12px 18px;"
        "font-size:16px;"
        "font-weight:bold;"
        "cursor:pointer;"
        "}"

        ".btn-primary{"
        "background:#1f2933;"
        "color:white;"
        "}"

        ".btn-secondary{"
        "background:#e5e7eb;"
        "color:#1f2933;"
        "}"

        ".btn-danger{"
        "background:#b42318;"
        "color:white;"
        "}"

        ".btn-success{"
        "background:#067647;"
        "color:white;"
        "}"

        ".btn:disabled{"
        "opacity:.4;"
        "cursor:not-allowed;"
        "}"

        "form{"
        "margin:0;"
        "}"

        ".state-badge{"
        "display:inline-block;"
        "padding:8px 16px;"
        "border-radius:999px;"
        "font-size:22px;"
        "font-weight:bold;"
        "}"

        ".state-on{"
        "background:#d1fae5;"
        "color:#067647;"
        "}"

        ".state-off{"
        "background:#e5e7eb;"
        "color:#344054;"
        "}"

        ".btn-active-on{"
        "background:#067647;"
        "color:white;"
        "}"

        ".btn-active-off{"
        "background:#344054;"
        "color:white;"
        "}"

        ".btn-inactive{"
        "background:#e5e7eb;"
        "color:#667085;"
        "}"

        "</style>"
        "</head>"
        "<body>"
        "<div class='container'>"
        "<header>"
        "<h1>Smart Greenhouse</h1>"
        "<div class='subtitle'>Local Monitoring Dashboard</div>"
        "</header>"
    ));


    if (!dashboardDataAvailable)
    {
        sendHtml(F(
            "<div class='card'>"
            "<div class='value'>Waiting for sensor data...</div>"
            "</div>"
        ));
    }
    else
    {
        const VPDProfile& profile = getActiveVPDProfile();


        /* Environment */
        sendHtml(F(
            "<div class='section-title'>Environment</div>"
            "<div class='grid'>"
            "<div class='card'>"
            "<div class='label'>Temperature</div>"
            "<div class='value'>"
        ));

        if (latestSensorData.environmentalSensorValid)
        {
            sendHtml(String(latestSensorData.temperature, 2));
            sendHtml(F(" &deg;C"));
        }
        else
        {
            sendHtml(F("INVALID"));
        }

        sendHtml(F(
            "</div></div>"
            "<div class='card'>"
            "<div class='label'>Humidity</div>"
            "<div class='value'>"
        ));

        if (latestSensorData.environmentalSensorValid)
        {
            sendHtml(String(latestSensorData.humidity, 2));
            sendHtml(F(" %"));
        }
        else
        {
            sendHtml(F("INVALID"));
        }

        sendHtml(F(
            "</div></div>"
            "<div class='card'>"
            "<div class='label'>Pressure</div>"
            "<div class='value'>"
        ));

        if (latestSensorData.environmentalSensorValid)
        {
            sendHtml(String(latestSensorData.pressure, 2));
            sendHtml(F(" hPa"));
        }
        else
        {
            sendHtml(F("INVALID"));
        }

        sendHtml(F(
            "</div></div>"
            "<div class='card'>"
            "<div class='label'>Soil Moisture</div>"
            "<div class='value'>"
        ));

        sendHtml(String(latestSensorData.soilPercent, 1));

        sendHtml(F(
            " %"
            "</div></div>"
            "</div>"
        ));


        /* VPD */
        sendHtml(F(
            "<div class='section-title'>VPD</div>"
            "<div class='grid'>"
            "<div class='card'>"
            "<div class='label'>Air VPD</div>"
            "<div class='value'>"
        ));

        if (!isnan(latestAirVPD))
        {
            sendHtml(String(latestAirVPD, 2));
            sendHtml(F(" kPa"));
        }
        else
        {
            sendHtml(F("INVALID"));
        }

        sendHtml(F(
            "</div></div>"
            "<div class='card'>"
            "<div class='label'>Target VPD</div>"
            "<div class='value'>"
        ));

        sendHtml(String(profile.targetVPD, 2));

        sendHtml(F(
            " kPa"
            "</div></div>"
            "<div class='card'>"
            "<div class='label'>VPD Range</div>"
            "<div class='value'>"
        ));

        sendHtml(String(profile.minVPD, 2));
        sendHtml(F(" - "));
        sendHtml(String(profile.maxVPD, 2));

        sendHtml(F(
            " kPa"
            "</div></div>"
            "<div class='card'>"
            "<div class='label'>VPD Status</div>"
            "<div class='value status'>"
        ));

        if (latestControlState.controlValid)
        {
            sendHtml(String(vpdStatusToString(latestControlState.vpdStatus)));
        }
        else
        {
            sendHtml(F("INVALID"));
        }

        sendHtml(F(
            "</div></div>"
            "</div>"
        ));


        /* Controller */
        sendHtml(F(
            "<div class='section-title'>Controller</div>"
            "<div class='grid'>"
            "<div class='card'>"
            "<div class='label'>Operating Mode</div>"
            "<div class='value'>"
        ));

        sendHtml(String(controlModeToString(latestControlState.controlMode)));

        sendHtml(F(
            "</div></div>"
            "<div class='card'>"
            "<div class='label'>Growth Stage</div>"
            "<div class='value'>"
        ));

        sendHtml(String(profile.name));

        sendHtml(F(
            "</div></div>"
            "<div class='card'>"
            "<div class='label'>Humidifier</div>"
        ));

        if (latestControlState.humidifierOn)
        {
            sendHtml(F(
                "<div class='state-badge state-on'>"
                "ON"
                "</div>"
            ));
        }
        else
        {
            sendHtml(F(
                "<div class='state-badge state-off'>"
                "OFF"
                "</div>"
            ));
        }

        sendHtml(F(
            "</div>"
            "<div class='card'>"
            "<div class='label'>Control</div>"
            "<div class='value'>"
        ));

        sendHtml(latestControlState.controlValid ? F("ACTIVE") : F("FAIL-SAFE"));

        sendHtml(F(
            "</div></div>"
            "<div class='card'>"
            "<div class='label'>Timing Lock</div>"
            "<div class='value'>"
        ));

        sendHtml(latestControlState.relayTimingLockout ? F("ACTIVE") : F("OFF"));

        sendHtml(F(
            "</div></div>"
            "<div class='card'>"
            "<div class='label'>RH Safety</div>"
            "<div class='value'>"
        ));

        sendHtml(latestControlState.humiditySafetyActive ? F("ACTIVE") : F("OFF"));

        sendHtml(F(
            "</div></div>"
            "</div>"
        ));


        /*
        * Control Actions.
        */
        sendHtml(F(
            "<div class='section-title'>Control Actions</div>"
            "<div class='card'>"
            "<div class='label'>Operating Mode</div>"
            "<div class='controls'>"
        ));


        /*
        * Highlight the currently selected operating mode.
        */
        if (latestControlState.controlMode == ControlMode::Automatic)
        {
            sendHtml(F(
                "<form method='POST' action='/control/mode/auto'>"
                "<button class='btn btn-primary' type='submit'>AUTO</button>"
                "</form>"

                "<form method='POST' action='/control/mode/manual'>"
                "<button class='btn btn-secondary' type='submit'>MANUAL</button>"
                "</form>"
            ));
        }
        else
        {
            sendHtml(F(
                "<form method='POST' action='/control/mode/auto'>"
                "<button class='btn btn-secondary' type='submit'>AUTO</button>"
                "</form>"

                "<form method='POST' action='/control/mode/manual'>"
                "<button class='btn btn-primary' type='submit'>MANUAL</button>"
                "</form>"
            ));
        }


        sendHtml(F(
            "</div>"
            "</div>"

            "<div class='card' style='margin-top:14px;'>"
            "<div class='label'>Manual Humidifier Control</div>"
            "<div class='controls'>"
        ));

        if (latestControlState.controlMode ==
            ControlMode::Manual)
        {
            /*
            * Manual mode is active.
            *
            * Highlight the button that matches
            * the actual humidifier output state.
            */
            if (latestControlState.humidifierOn)
            {
                sendHtml(F(
                    "<form method='POST' action='/control/humidifier/on'>"
                    "<button class='btn btn-active-on' type='submit'>"
                    "ON"
                    "</button>"
                    "</form>"

                    "<form method='POST' action='/control/humidifier/off'>"
                    "<button class='btn btn-inactive' type='submit'>"
                    "OFF"
                    "</button>"
                    "</form>"
                ));
            }
            else
            {
                sendHtml(F(
                    "<form method='POST' action='/control/humidifier/on'>"
                    "<button class='btn btn-inactive' type='submit'>"
                    "ON"
                    "</button>"
                    "</form>"

                    "<form method='POST' action='/control/humidifier/off'>"
                    "<button class='btn btn-active-off' type='submit'>"
                    "OFF"
                    "</button>"
                    "</form>"
                ));
            }
        }
        else
        {
            /*
            * Manual controls are disabled while
            * Automatic mode is active.
            */
            sendHtml(F(
                "<button class='btn btn-inactive' disabled>"
                "ON"
                "</button>"

                "<button class='btn btn-inactive' disabled>"
                "OFF"
                "</button>"
            ));
        }

        sendHtml(F(
            "</div>"
            "</div>"
        ));


        /* Network */
        sendHtml(F(
            "<div class='section-title'>Network</div>"
            "<div class='grid'>"
            "<div class='card'>"
            "<div class='label'>Wi-Fi</div>"
            "<div class='value'>"
        ));

        sendHtml(WiFi.status() == WL_CONNECTED ? F("CONNECTED") : F("OFFLINE"));

        sendHtml(F("</div></div>"));

        if (WiFi.status() == WL_CONNECTED)
        {
            sendHtml(F(
                "<div class='card'>"
                "<div class='label'>IP Address</div>"
                "<div class='value'>"
            ));

            sendHtml(WiFi.localIP().toString());

            sendHtml(F(
                "</div></div>"
                "<div class='card'>"
                "<div class='label'>Wi-Fi Signal</div>"
                "<div class='value'>"
            ));

            sendHtml(String(WiFi.RSSI()));

            sendHtml(F(
                " dBm"
                "</div></div>"
            ));
        }

        sendHtml(F("</div>"));


        /* Footer */
        sendHtml(F(
            "<div class='footer'>"
            "Last sensor update: "
        ));

        sendHtml(String((millis() - lastDataUpdateTime) / 1000));

        sendHtml(F(
            " seconds ago"
            "<br>"
            "Dashboard refreshes automatically every 3 seconds."
            "</div>"
        ));
    }

    sendHtml(F(
        "</div>"
        "</body>"
        "</html>"
    ));

    /* Final zero-length chunk terminates the HTTP chunked response. */
    server.sendContent(String());
}


/*
 * Handle invalid URLs.
 */
static void handleNotFound()
{
    server.send(
        404,
        "text/plain",
        "404 - Page not found"
    );
}


/*
 * Redirect the browser back to the main dashboard
 * after a control command has been processed.
 */
static void redirectToDashboard()
{
    server.sendHeader(
        "Location",
        "/",
        true
    );

    server.send(
        303,
        "text/plain",
        ""
    );
}


/*
 * Switch controller to Automatic mode.
 */
static void handleModeAutomatic()
{
    setControlMode(
        ControlMode::Automatic
    );

    Serial.println();
    Serial.println(
        "WEB COMMAND: Operating mode -> AUTO"
    );

    redirectToDashboard();
}


/*
 * Switch controller to Manual mode.
 */
static void handleModeManual()
{
    setControlMode(
        ControlMode::Manual
    );

    Serial.println();
    Serial.println(
        "WEB COMMAND: Operating mode -> MANUAL"
    );

    redirectToDashboard();
}


/*
 * Request humidifier ON while operating in Manual mode.
 *
 * The web server never writes the relay GPIO directly. The request still
 * passes through the control module and its safety protections.
 */
static void handleManualHumidifierOn()
{
    if (getControlMode() != ControlMode::Manual)
    {
        server.send(
            409,
            "text/plain",
            "Manual humidifier control is available only in MANUAL mode."
        );

        return;
    }

    setManualHumidifierRequest(true);

    Serial.println();
    Serial.println(
        "WEB COMMAND: Manual humidifier request -> ON"
    );

    redirectToDashboard();
}


/*
 * Request humidifier OFF while operating in Manual mode.
 */
static void handleManualHumidifierOff()
{
    if (getControlMode() != ControlMode::Manual)
    {
        server.send(
            409,
            "text/plain",
            "Manual humidifier control is available only in MANUAL mode."
        );

        return;
    }

    setManualHumidifierRequest(false);

    Serial.println();
    Serial.println(
        "WEB COMMAND: Manual humidifier request -> OFF"
    );

    redirectToDashboard();
}


void initializeWebServer()
{
    /* Configure routes only once. */
    if (!routesConfigured)
    {
        server.on(
            "/",
            HTTP_GET,
            handleDashboard
        );

        server.on(
            "/control/mode/auto",
            HTTP_POST,
            handleModeAutomatic
        );

        server.on(
            "/control/mode/manual",
            HTTP_POST,
            handleModeManual
        );

        server.on(
            "/control/humidifier/on",
            HTTP_POST,
            handleManualHumidifierOn
        );

        server.on(
            "/control/humidifier/off",
            HTTP_POST,
            handleManualHumidifierOff
        );

        server.onNotFound(
            handleNotFound
        );

        routesConfigured = true;
    }

    startWebServerIfPossible();

    if (!webServerStarted)
    {
        Serial.println(
            "Web dashboard : WAITING FOR WI-FI"
        );
    }
}


void updateWebServerData(
    const SensorData& sensorData,
    float airVPD,
    const ControlState& controlState
)
{
    latestSensorData = sensorData;
    latestAirVPD = airVPD;
    latestControlState = controlState;
    lastDataUpdateTime = millis();
    dashboardDataAvailable = true;
}


void processWebServer()
{
    /*
     * This also handles the case where the ESP8266 initially booted without
     * Wi-Fi and connected later through provisioning.
     */
    startWebServerIfPossible();

    if (!webServerStarted)
    {
        return;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        return;
    }

    server.handleClient();
}
