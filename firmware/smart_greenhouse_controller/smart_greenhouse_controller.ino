#include <Arduino.h>

#include "config.h"
#include "wifi_provisioning.h"


void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);

    delay(500);

    Serial.println();
    Serial.println("================================");
    Serial.println(" Smart Greenhouse Controller");
    Serial.println(" SW-07 Wi-Fi Provisioning");
    Serial.println("================================");
    Serial.println();


    const bool wifiConnected =
        initializeWiFiProvisioning();


    Serial.println();

    if (wifiConnected)
    {
        Serial.println(
            "SW-07 Wi-Fi initialization completed."
        );
    }
    else
    {
        Serial.println(
            "Wi-Fi unavailable."
        );

        Serial.println(
            "Controller remains operational offline."
        );
    }
}


void loop()
{
    /*
     * Print current Wi-Fi status every 10 seconds.
     */
    static unsigned long lastStatusTime = 0;

    if (millis() - lastStatusTime >= 10000UL)
    {
        lastStatusTime = millis();

        printWiFiStatus();
    }
}