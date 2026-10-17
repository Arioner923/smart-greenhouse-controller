#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>

#include "config.h"
#include "wifi_provisioning.h"


/*
 * WiFiManager instance used by this module.
 */
static WiFiManager wifiManager;


/*
 * Called when the device enters configuration mode.
 */
static void onConfigPortalStarted(WiFiManager* manager)
{
    Serial.println();
    Serial.println("================================");
    Serial.println(" Wi-Fi Configuration Portal");
    Serial.println("================================");

    Serial.print("Setup network : ");
    Serial.println(
        manager->getConfigPortalSSID()
    );

    Serial.print("Portal IP     : ");
    Serial.println(
        WiFi.softAPIP()
    );

    Serial.println();
    Serial.println(
        "Connect to the setup network"
    );

    Serial.println(
        "and configure Wi-Fi from your"
    );

    Serial.println(
        "phone or computer."
    );

    Serial.println();
}


bool initializeWiFiProvisioning()
{
    Serial.println();
    Serial.println("Initializing Wi-Fi...");

    /*
     * Use station mode for normal operation.
     */
    WiFi.mode(WIFI_STA);


    /*
     * Prevent indefinite blocking.
     */
    wifiManager.setConnectTimeout(
        WIFI_CONNECT_TIMEOUT_SECONDS
    );

    wifiManager.setConfigPortalTimeout(
        WIFI_CONFIG_PORTAL_TIMEOUT_SECONDS
    );


    /*
     * Called only when configuration portal
     * mode is entered.
     */
    wifiManager.setAPCallback(
        onConfigPortalStarted
    );


    /*
     * Attempt connection using saved credentials.
     *
     * If connection fails, WiFiManager creates:
     *
     * SmartGreenhouse-Setup
     *
     * and starts the captive portal.
     */
    const bool connected =
        wifiManager.autoConnect(
            WIFI_SETUP_AP_NAME,
            WIFI_SETUP_AP_PASSWORD
        );


    if (!connected)
    {
        Serial.println();
        Serial.println(
            "Wi-Fi connection failed or portal timed out."
        );

        Serial.println(
            "Controller will continue offline."
        );

        return false;
    }


    Serial.println();
    Serial.println("Wi-Fi connection: PASS");

    printWiFiStatus();

    return true;
}


bool isWiFiConnected()
{
    return WiFi.status() == WL_CONNECTED;
}


void printWiFiStatus()
{
    if (!isWiFiConnected())
    {
        Serial.println("Wi-Fi Status  : DISCONNECTED");
        return;
    }

    Serial.println("--------------------------------");

    Serial.print("Wi-Fi Status  : CONNECTED");
    Serial.println();

    Serial.print("SSID          : ");
    Serial.println(WiFi.SSID());

    Serial.print("IP Address    : ");
    Serial.println(WiFi.localIP());

    Serial.print("Signal RSSI   : ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    Serial.println("--------------------------------");
}


void clearWiFiCredentials()
{
    Serial.println();
    Serial.println(
        "Clearing saved Wi-Fi credentials..."
    );

    wifiManager.resetSettings();

    Serial.println(
        "Saved Wi-Fi credentials cleared."
    );
}