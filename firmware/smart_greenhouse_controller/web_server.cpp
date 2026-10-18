#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <math.h>

#include "types.h"
#include "control.h"
#include "web_server.h"
#include "dashboard_page.h"
#include "dashboard_script.h"
#include "soil_moisture.h"


/*
 * Smart Greenhouse Controller
 * Local Web Dashboard + Web Control
 *
 * Static dashboard HTML/CSS is stored in dashboard_page.h (PROGMEM).
 * Browser-side polling logic is stored in dashboard_script.h (PROGMEM).
 * Dynamic values, JSON status data and HTTP control routes remain here.
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

    /*
     * Send the static dashboard shell directly from flash.
     */
    server.sendContent_P(DASHBOARD_PAGE_START);
    yield();


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
            "<div class='value' id='temperatureValue'>"
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
            "<div class='value' id='humidityValue'>"
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
            "<div class='value' id='pressureValue'>"
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
            "<div class='value' id='soilValue'>"
        ));

        sendHtml(
            String(
                latestSensorData.soilPercent,
                1
            )
        );

        sendHtml(F(
            " %"
            "</div>"
            "<div style='margin-top:10px;'>"
        ));

        switch (latestSensorData.soilStatus)
        {
            case SoilMoistureStatus::VeryDry:
                sendHtml(F(
                    "<div id='soilStatusBadge' class='soil-badge soil-very-dry'>"
                ));
                break;

            case SoilMoistureStatus::Dry:
                sendHtml(F(
                    "<div id='soilStatusBadge' class='soil-badge soil-dry'>"
                ));
                break;

            case SoilMoistureStatus::Good:
                sendHtml(F(
                    "<div id='soilStatusBadge' class='soil-badge soil-good'>"
                ));
                break;

            case SoilMoistureStatus::Wet:
                sendHtml(F(
                    "<div id='soilStatusBadge' class='soil-badge soil-wet'>"
                ));
                break;

            case SoilMoistureStatus::VeryWet:
                sendHtml(F(
                    "<div id='soilStatusBadge' class='soil-badge soil-very-wet'>"
                ));
                break;
        }

        sendHtml(
            String(
                soilMoistureStatusToString(
                    latestSensorData.soilStatus
                )
            )
        );

        sendHtml(F(
            "</div>"
            "</div>"
            "</div>"
            "</div>"
        ));


        /* VPD */
        sendHtml(F(
            "<div class='section-title'>VPD</div>"
            "<div class='grid'>"
            "<div class='card'>"
            "<div class='label'>Air VPD</div>"
            "<div class='value' id='airVpdValue'>"
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
            "<div class='value' id='targetVpdValue'>"
        ));

        sendHtml(String(profile.targetVPD, 2));

        sendHtml(F(
            " kPa"
            "</div></div>"
            "<div class='card'>"
            "<div class='label'>VPD Range</div>"
            "<div class='value' id='vpdRangeValue'>"
        ));

        sendHtml(String(profile.minVPD, 2));
        sendHtml(F(" - "));
        sendHtml(String(profile.maxVPD, 2));

        sendHtml(F(
            " kPa"
            "</div></div>"
            "<div class='card'>"
            "<div class='label'>VPD Status</div>"
            "<div class='value status' id='vpdStatusValue'>"
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
            "<div class='value' id='modeValue'>"
        ));

        sendHtml(
            String(
                controlModeToString(
                    getControlMode()
                )
            )
        );
        
        sendHtml(F(
            "</div></div>"
            "<div class='card'>"
            "<div class='label'>Growth Stage</div>"
            "<div class='value' id='growthStageValue'>"
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
                "<div id='humidifierBadge' "
                "class='state-badge state-on'>"
                "<span id='humidifierValue'>ON</span>"
                "</div>"
            ));
        }
        else
        {
            sendHtml(F(
                "<div id='humidifierBadge' "
                "class='state-badge state-off'>"
                "<span id='humidifierValue'>OFF</span>"
                "</div>"
            ));
        }

        sendHtml(F(
            "</div>"
            "<div class='card'>"
            "<div class='label'>Control</div>"
            "<div class='value' id='controlValue'>"
        ));

        sendHtml(latestControlState.controlValid ? F("ACTIVE") : F("FAIL-SAFE"));

        sendHtml(F(
            "</div></div>"
            "<div class='card'>"
            "<div class='label'>Timing Lock</div>"
            "<div class='value' id='timingLockValue'>"
        ));

        sendHtml(latestControlState.relayTimingLockout ? F("ACTIVE") : F("OFF"));

        sendHtml(F(
            "</div></div>"
            "<div class='card'>"
            "<div class='label'>RH Safety</div>"
            "<div class='value' id='rhSafetyValue'>"
        ));

        sendHtml(latestControlState.humiditySafetyActive ? F("ACTIVE") : F("OFF"));

        sendHtml(F(
            "</div></div>"
            "</div>"
        ));


        /* =====================================================
         * Control Actions
         * =====================================================
         *
         * The form structure is always rendered, regardless of mode.
         * JavaScript only changes disabled/highlight state. This keeps
         * the DOM stable when AUTO and MANUAL are switched live.
         */
        sendHtml(F(
            "<div class='section-title'>Control Actions</div>"
            "<div class='card'>"
            "<div class='label'>Operating Mode</div>"
            "<div class='controls'>"
        ));

        if (getControlMode() == ControlMode::Automatic)
        {
            sendHtml(F(
                "<form method='POST' action='/control/mode/auto'>"
                "<button id='autoModeButton' class='btn btn-primary' "
                "type='submit'>AUTO</button>"
                "</form>"

                "<form method='POST' action='/control/mode/manual'>"
                "<button id='manualModeButton' class='btn btn-secondary' "
                "type='submit'>MANUAL</button>"
                "</form>"
            ));
        }
        else
        {
            sendHtml(F(
                "<form method='POST' action='/control/mode/auto'>"
                "<button id='autoModeButton' class='btn btn-secondary' "
                "type='submit'>AUTO</button>"
                "</form>"

                "<form method='POST' action='/control/mode/manual'>"
                "<button id='manualModeButton' class='btn btn-primary' "
                "type='submit'>MANUAL</button>"
                "</form>"
            ));
        }

        sendHtml(F(
            "</div>"
            "</div>"

            "<div class='card' style='margin-top:14px;'>"
            "<div class='label'>Manual Humidifier Control</div>"
            "<div class='controls'>"
            "<form method='POST' action='/control/humidifier/on'>"
        ));

        if (getControlMode() == ControlMode::Manual)
        {
            if (latestControlState.humidifierOn)
            {
                sendHtml(F(
                    "<button id='humidifierOnButton' "
                    "class='btn btn-active-on' type='submit'>ON</button>"
                ));
            }
            else
            {
                sendHtml(F(
                    "<button id='humidifierOnButton' "
                    "class='btn btn-inactive' type='submit'>ON</button>"
                ));
            }
        }
        else
        {
            sendHtml(F(
                "<button id='humidifierOnButton' "
                "class='btn btn-inactive' type='submit' disabled>"
                "ON</button>"
            ));
        }

        sendHtml(F(
            "</form>"
            "<form method='POST' action='/control/humidifier/off'>"
        ));

        if (getControlMode() == ControlMode::Manual)
        {
            if (latestControlState.humidifierOn)
            {
                sendHtml(F(
                    "<button id='humidifierOffButton' "
                    "class='btn btn-inactive' type='submit'>OFF</button>"
                ));
            }
            else
            {
                sendHtml(F(
                    "<button id='humidifierOffButton' "
                    "class='btn btn-active-off' type='submit'>OFF</button>"
                ));
            }
        }
        else
        {
            sendHtml(F(
                "<button id='humidifierOffButton' "
                "class='btn btn-inactive' type='submit' disabled>"
                "OFF</button>"
            ));
        }

        sendHtml(F(
            "</form>"
            "</div>"
            "</div>"
        ));


        /* Network */
        sendHtml(F(
            "<div class='section-title'>Network</div>"
            "<div class='grid'>"
            "<div class='card'>"
            "<div class='label'>Wi-Fi</div>"
            "<div class='value' id='wifiValue'>"
        ));

        sendHtml(WiFi.status() == WL_CONNECTED ? F("CONNECTED") : F("OFFLINE"));

        sendHtml(F("</div></div>"));

        if (WiFi.status() == WL_CONNECTED)
        {
            sendHtml(F(
                "<div class='card'>"
                "<div class='label'>IP Address</div>"
                "<div class='value' id='ipValue'>"
            ));

            sendHtml(WiFi.localIP().toString());

            sendHtml(F(
                "</div></div>"
                "<div class='card'>"
                "<div class='label'>Wi-Fi Signal</div>"
                "<div class='value' id='rssiValue'>"
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
            "<span id='lastUpdateValue'>"
        ));

        sendHtml(String((millis() - lastDataUpdateTime) / 1000));
        sendHtml(F(
            "</span>"
            " seconds ago"
            "<br>"
            "Live data updates automatically every 2 seconds."
            "</div>"
        ));
    }

    /*
     * Close the static dashboard shell from flash.
     */
    server.sendContent_P(DASHBOARD_PAGE_END);
    yield();

    /* Final zero-length chunk terminates the HTTP chunked response. */
    server.sendContent(String());
}

/*
 * Serve the browser-side dashboard JavaScript from flash.
 *
 * Keeping the script as a separate HTTP resource makes dashboard_page.h
 * easier to maintain without requiring a filesystem such as LittleFS.
 */
static void handleDashboardScript()
{
    server.sendHeader(
        "Cache-Control",
        "no-store"
    );

    server.setContentLength(
        CONTENT_LENGTH_UNKNOWN
    );

    server.send(
        200,
        "application/javascript",
        ""
    );

    server.sendContent_P(
        DASHBOARD_SCRIPT
    );

    server.sendContent(
        String()
    );
}


/*
 * Return the latest greenhouse state as JSON.
 *
 * This endpoint is used by the dashboard to update
 * live values without reloading the entire HTML page.
 */
static void handleApiStatus()
{
    if (!dashboardDataAvailable)
    {
        server.send(
            503,
            "application/json",
            "{\"available\":false}"
        );

        return;
    }


    const VPDProfile& profile =
        getActiveVPDProfile();


    String json;

    /*
     * Reserve enough space to reduce dynamic
     * String reallocations on the ESP8266.
     */
    json.reserve(700);


    json += F("{");

    json += F("\"available\":true,");


    /*
     * Environmental measurements.
     */
    json += F("\"temperature\":");

    if (latestSensorData.environmentalSensorValid)
    {
        json += String(
            latestSensorData.temperature,
            2
        );
    }
    else
    {
        json += F("null");
    }


    json += F(",\"humidity\":");

    if (latestSensorData.environmentalSensorValid)
    {
        json += String(
            latestSensorData.humidity,
            2
        );
    }
    else
    {
        json += F("null");
    }


    json += F(",\"pressure\":");

    if (latestSensorData.environmentalSensorValid)
    {
        json += String(
            latestSensorData.pressure,
            2
        );
    }
    else
    {
        json += F("null");
    }


    /*
     * Soil moisture.
     */
    json += F(",\"soilPercent\":");
    json += String(
        latestSensorData.soilPercent,
        1
    );

    json += F(",\"soilStatus\":\"");
    json += soilMoistureStatusToString(
        latestSensorData.soilStatus
    );
    json += F("\"");


    /*
     * VPD.
     */
    json += F(",\"airVPD\":");

    if (!isnan(latestAirVPD))
    {
        json += String(
            latestAirVPD,
            2
        );
    }
    else
    {
        json += F("null");
    }


    json += F(",\"targetVPD\":");
    json += String(
        profile.targetVPD,
        2
    );

    json += F(",\"minVPD\":");
    json += String(
        profile.minVPD,
        2
    );

    json += F(",\"maxVPD\":");
    json += String(
        profile.maxVPD,
        2
    );


    json += F(",\"vpdStatus\":\"");

    if (latestControlState.controlValid)
    {
        json += vpdStatusToString(
            latestControlState.vpdStatus
        );
    }
    else
    {
        json += F("INVALID");
    }

    json += F("\"");


    /*
     * Controller state.
     */
    json += F(",\"mode\":\"");
    json += controlModeToString(
        getControlMode()
    );
    json += F("\"");


    json += F(",\"growthStage\":\"");
    json += profile.name;
    json += F("\"");


    json += F(",\"humidifier\":");
    json += latestControlState.humidifierOn
        ? F("true")
        : F("false");


    json += F(",\"controlValid\":");
    json += latestControlState.controlValid
        ? F("true")
        : F("false");


    json += F(",\"timingLock\":");
    json += latestControlState.relayTimingLockout
        ? F("true")
        : F("false");


    json += F(",\"rhSafety\":");
    json += latestControlState.humiditySafetyActive
        ? F("true")
        : F("false");


    /*
     * Network information.
     */
    json += F(",\"wifiConnected\":");
    json += WiFi.status() == WL_CONNECTED
        ? F("true")
        : F("false");


    json += F(",\"ip\":\"");

    if (WiFi.status() == WL_CONNECTED)
    {
        json += WiFi.localIP().toString();
    }

    json += F("\"");


    json += F(",\"rssi\":");

    if (WiFi.status() == WL_CONNECTED)
    {
        json += String(
            WiFi.RSSI()
        );
    }
    else
    {
        json += F("null");
    }


    /*
     * Age of the latest sensor update.
     */
    json += F(",\"lastUpdateSeconds\":");
    json += String(
        (millis() - lastDataUpdateTime) / 1000
    );


    json += F("}");


    server.sendHeader(
        "Cache-Control",
        "no-store"
    );

    server.send(
        200,
        "application/json",
        json
    );
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

        /*
         * Browser-side live dashboard script.
         */
        server.on(
            "/dashboard.js",
            HTTP_GET,
            handleDashboardScript
        );

        /*
         * Live dashboard data API.
         */
        server.on(
            "/api/status",
            HTTP_GET,
            handleApiStatus
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
