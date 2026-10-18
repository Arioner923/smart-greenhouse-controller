#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>

#include "config.h"
#include "wifi_provisioning.h"


/*
 * WiFiManager instance used by this module.
 */
static WiFiManager wifiManager;

/*
 * Tracks whether the local mDNS responder
 * is currently active.
 */
static bool mdnsStarted = false;


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

/*
 * Start the local mDNS service when Wi-Fi
 * connectivity becomes available.
 *
 * This allows the dashboard to be reached using:
 *
 * http://smartgreenhouse.local/
 *
 * instead of requiring the user to know the
 * dynamically assigned IP address.
 */
static void startMDNSIfPossible()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return;
    }

    if (mdnsStarted)
    {
        return;
    }

    if (!MDNS.begin(WIFI_MDNS_HOSTNAME))
    {
        Serial.println(
            "mDNS initialization: FAILED"
        );

        return;
    }

    /*
     * Advertise the local HTTP dashboard.
     */
    MDNS.addService(
        "http",
        "tcp",
        80
    );

    mdnsStarted = true;

    Serial.println();
    Serial.println("mDNS initialization: PASS");

    Serial.print("Local hostname     : http://");
    Serial.print(WIFI_MDNS_HOSTNAME);
    Serial.println(".local/");
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
    * Friendly DHCP/device hostname.
    */
    WiFi.hostname(
        WIFI_MDNS_HOSTNAME
    );

    /*
     * Keep the configuration portal non-blocking.
     *
     * This allows the greenhouse controller to
     * continue monitoring sensors and controlling
     * the relay even when Wi-Fi is unavailable.
     */
    wifiManager.setConfigPortalBlocking(false);


    /*
     * Prevent long connection attempts.
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
     * If connection fails, WiFiManager creates
     * the SmartGreenhouse-Setup access point.
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
            "Wi-Fi not connected."
        );

        Serial.println(
            "Configuration portal is running."
        );

        Serial.println(
            "Controller continues offline."
        );

        return false;
    }


    Serial.println();
    Serial.println("Wi-Fi connection: PASS");

    printWiFiStatus();

    /*
    * Start local hostname service.
    */
    startMDNSIfPossible();

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

void processWiFiProvisioning()
{
    /*
     * Required for non-blocking WiFiManager.
     */
    wifiManager.process();


    /*
     * Wi-Fi connected or reconnected.
     */
    if (WiFi.status() == WL_CONNECTED)
    {
        /*
         * Start mDNS if this connection became
         * available after boot or provisioning.
         */
        startMDNSIfPossible();


        /*
         * Keep the mDNS responder active.
         */
        if (mdnsStarted)
        {
            MDNS.update();
        }

        return;
    }


    /*
     * Wi-Fi was lost.
     *
     * Stop the old mDNS responder so that it can
     * be cleanly recreated after reconnection.
     */
    if (mdnsStarted)
    {
        MDNS.close();

        mdnsStarted = false;

        Serial.println(
            "mDNS stopped: Wi-Fi unavailable"
        );
    }
}