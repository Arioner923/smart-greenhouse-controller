/*
 * Smart Greenhouse Controller
 * I2C Bus Scanner
 *
 * Controller:
 *   WeMos D1 R2 V2.1.0 / ESP8266
 *
 * I2C wiring:
 *   SDA -> D2 / GPIO4
 *   SCL -> D1 / GPIO5
 *
 * Purpose:
 *   Scan the I2C bus and report the addresses of
 *   connected I2C devices.
 *
 * Initial target:
 *   GY-BME/P280 environmental sensor.
 */

#include <Wire.h>

// ESP8266 I2C pins
const uint8_t SDA_PIN = D2;
const uint8_t SCL_PIN = D1;

void setup()
{
    // Start serial communication for diagnostic output.
    Serial.begin(74880);

    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println(" Smart Greenhouse Controller");
    Serial.println(" I2C Scanner");
    Serial.println("================================");
    Serial.println();

    // Start I2C communication using the selected ESP8266 pins.
    Wire.begin(SDA_PIN, SCL_PIN);

    Serial.println("I2C bus initialized.");
    Serial.println("SDA: D2 / GPIO4");
    Serial.println("SCL: D1 / GPIO5");
    Serial.println();
}

void loop()
{
    uint8_t error;
    uint8_t address;
    uint8_t devicesFound = 0;

    Serial.println("Scanning I2C bus...");

    // Valid 7-bit I2C addresses normally range from 1 to 126.
    for (address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            Serial.print("I2C device found at address 0x");

            if (address < 16)
            {
                Serial.print("0");
            }

            Serial.println(address, HEX);
            devicesFound++;
        }
    }

    if (devicesFound == 0)
    {
        Serial.println("No I2C devices found.");
    }
    else
    {
        Serial.print("Scan complete. Devices found: ");
        Serial.println(devicesFound);
    }

    Serial.println();
    Serial.println("--------------------------------");
    Serial.println();

    // Repeat the scan every 5 seconds.
    delay(5000);
}