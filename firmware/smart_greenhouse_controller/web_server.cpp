#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <math.h>

#include "types.h"
#include "control.h"
#include "web_server.h"


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
 *
 * The dashboard displays a snapshot of the most
 * recent control cycle.
 */
static SensorData latestSensorData;
static ControlState latestControlState;

static float latestAirVPD = NAN;

static bool dashboardDataAvailable = false;
static bool webServerStarted = false;
static bool routesConfigured = false;

static unsigned long lastDataUpdateTime = 0;


/*
 * Start the local HTTP server when Wi-Fi
 * is available.
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
 */
static void handleDashboard()
{
    String page;

    /*
     * Reserve memory in advance to reduce
     * dynamic String memory fragmentation.
     */
    page.reserve(10000);


    page += F(
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' "
        "content='width=device-width,initial-scale=1'>"

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
        "grid-template-columns:"
        "repeat(auto-fit,minmax(190px,1fr));"
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

        "</style>"
        "</head>"

        "<body>"
        "<div class='container'>"

        "<header>"
        "<h1>Smart Greenhouse</h1>"
        "<div class='subtitle'>"
        "Local Monitoring Dashboard"
        "</div>"
        "</header>"
    );


    /*
     * No measurement cycle has completed yet.
     */
    if (!dashboardDataAvailable)
    {
        page += F(
            "<div class='card'>"
            "<div class='value'>"
            "Waiting for sensor data..."
            "</div>"
            "</div>"
        );
    }
    else
    {
        const VPDProfile& profile =
            getActiveVPDProfile();


        /*
         * Environmental measurements.
         */
        page += F(
            "<div class='section-title'>"
            "Environment"
            "</div>"

            "<div class='grid'>"
        );


        page += F(
            "<div class='card'>"
            "<div class='label'>Temperature</div>"
            "<div class='value'>"
        );

        if (latestSensorData.environmentalSensorValid)
        {
            page += String(
                latestSensorData.temperature,
                2
            );

            page += F(" &deg;C");
        }
        else
        {
            page += F("INVALID");
        }

        page += F(
            "</div>"
            "</div>"
        );


        page += F(
            "<div class='card'>"
            "<div class='label'>Humidity</div>"
            "<div class='value'>"
        );

        if (latestSensorData.environmentalSensorValid)
        {
            page += String(
                latestSensorData.humidity,
                2
            );

            page += F(" %");
        }
        else
        {
            page += F("INVALID");
        }

        page += F(
            "</div>"
            "</div>"
        );


        page += F(
            "<div class='card'>"
            "<div class='label'>Pressure</div>"
            "<div class='value'>"
        );

        if (latestSensorData.environmentalSensorValid)
        {
            page += String(
                latestSensorData.pressure,
                2
            );

            page += F(" hPa");
        }
        else
        {
            page += F("INVALID");
        }

        page += F(
            "</div>"
            "</div>"
        );


        page += F(
            "<div class='card'>"
            "<div class='label'>Soil Moisture</div>"
            "<div class='value'>"
        );

        page += String(
            latestSensorData.soilPercent,
            1
        );

        page += F(
            " %"
            "</div>"
            "</div>"
        );


        page += F("</div>");


        /*
         * VPD section.
         */
        page += F(
            "<div class='section-title'>"
            "VPD"
            "</div>"

            "<div class='grid'>"
        );


        page += F(
            "<div class='card'>"
            "<div class='label'>Air VPD</div>"
            "<div class='value'>"
        );

        if (!isnan(latestAirVPD))
        {
            page += String(
                latestAirVPD,
                2
            );

            page += F(" kPa");
        }
        else
        {
            page += F("INVALID");
        }

        page += F(
            "</div>"
            "</div>"
        );


        page += F(
            "<div class='card'>"
            "<div class='label'>Target VPD</div>"
            "<div class='value'>"
        );

        page += String(
            profile.targetVPD,
            2
        );

        page += F(
            " kPa"
            "</div>"
            "</div>"
        );


        page += F(
            "<div class='card'>"
            "<div class='label'>VPD Range</div>"
            "<div class='value'>"
        );

        page += String(
            profile.minVPD,
            2
        );

        page += F(" - ");

        page += String(
            profile.maxVPD,
            2
        );

        page += F(
            " kPa"
            "</div>"
            "</div>"
        );


        page += F(
            "<div class='card'>"
            "<div class='label'>VPD Status</div>"
            "<div class='value status'>"
        );

        if (latestControlState.controlValid)
        {
            page +=
                vpdStatusToString(
                    latestControlState.vpdStatus
                );
        }
        else
        {
            page += F("INVALID");
        }

        page += F(
            "</div>"
            "</div>"
        );


        page += F("</div>");


        /*
         * Controller section.
         */
        page += F(
            "<div class='section-title'>"
            "Controller"
            "</div>"

            "<div class='grid'>"
        );


        page += F(
            "<div class='card'>"
            "<div class='label'>Operating Mode</div>"
            "<div class='value'>"
        );

        page +=
            controlModeToString(
                latestControlState.controlMode
            );

        page += F(
            "</div>"
            "</div>"
        );


        page += F(
            "<div class='card'>"
            "<div class='label'>Growth Stage</div>"
            "<div class='value'>"
        );

        page += profile.name;

        page += F(
            "</div>"
            "</div>"
        );


        page += F(
            "<div class='card'>"
            "<div class='label'>Humidifier</div>"
            "<div class='value'>"
        );

        page +=
            latestControlState.humidifierOn
                ? F("ON")
                : F("OFF");

        page += F(
            "</div>"
            "</div>"
        );


        page += F(
            "<div class='card'>"
            "<div class='label'>Control</div>"
            "<div class='value'>"
        );

        page +=
            latestControlState.controlValid
                ? F("ACTIVE")
                : F("FAIL-SAFE");

        page += F(
            "</div>"
            "</div>"
        );


        page += F(
            "<div class='card'>"
            "<div class='label'>Timing Lock</div>"
            "<div class='value'>"
        );

        page +=
            latestControlState.relayTimingLockout
                ? F("ACTIVE")
                : F("OFF");

        page += F(
            "</div>"
            "</div>"
        );


        page += F(
            "<div class='card'>"
            "<div class='label'>RH Safety</div>"
            "<div class='value'>"
        );

        page +=
            latestControlState.humiditySafetyActive
                ? F("ACTIVE")
                : F("OFF");

        page += F(
            "</div>"
            "</div>"
        );


        page += F("</div>");


        /*
         * Network section.
         */
        page += F(
            "<div class='section-title'>"
            "Network"
            "</div>"

            "<div class='grid'>"
        );


        page += F(
            "<div class='card'>"
            "<div class='label'>Wi-Fi</div>"
            "<div class='value'>"
        );

        page +=
            WiFi.status() == WL_CONNECTED
                ? F("CONNECTED")
                : F("OFFLINE");

        page += F(
            "</div>"
            "</div>"
        );


        if (WiFi.status() == WL_CONNECTED)
        {
            page += F(
                "<div class='card'>"
                "<div class='label'>IP Address</div>"
                "<div class='value'>"
            );

            page += WiFi.localIP().toString();

            page += F(
                "</div>"
                "</div>"
            );


            page += F(
                "<div class='card'>"
                "<div class='label'>Wi-Fi Signal</div>"
                "<div class='value'>"
            );

            page += String(WiFi.RSSI());

            page += F(
                " dBm"
                "</div>"
                "</div>"
            );
        }


        page += F("</div>");


        /*
         * Last measurement information.
         */
        page += F(
            "<div class='footer'>"
            "Last sensor update: "
        );

        page += String(
            (millis() - lastDataUpdateTime) / 1000
        );

        page += F(
            " seconds ago"
            "<br>"
            "Dashboard refreshes automatically every 3 seconds."
            "</div>"
        );
    }


    page += F(
        "</div>"
        "</body>"
        "</html>"
    );

    Serial.print("Dashboard HTML size : ");
    Serial.print(page.length());
    Serial.println(" bytes");

    Serial.print("Free heap           : ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");

    server.send(
        200,
        "text/html",
        page
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


void initializeWebServer()
{
    /*
     * Configure routes only once.
     */
    if (!routesConfigured)
    {
        server.on(
            "/",
            HTTP_GET,
            handleDashboard
        );

        server.onNotFound(
            handleNotFound
        );

        routesConfigured = true;
    }


    /*
     * Start immediately if Wi-Fi is already available.
     *
     * Otherwise processWebServer() will start it
     * automatically after Wi-Fi becomes available.
     */
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
    latestSensorData =
        sensorData;

    latestAirVPD =
        airVPD;

    latestControlState =
        controlState;

    lastDataUpdateTime =
        millis();

    dashboardDataAvailable =
        true;
}


void processWebServer()
{
    /*
     * This also handles the case where the ESP8266
     * initially booted without Wi-Fi and connected
     * later through provisioning.
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