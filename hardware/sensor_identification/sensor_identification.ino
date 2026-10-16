/*
 * Smart Greenhouse Controller
 * BME280 / BMP280 Identification Test
 *
 * Board:
 *   WeMos D1 R2 V2.1.0 / ESP8266
 *
 * Sensor I2C address detected previously:
 *   0x76
 *
 * Wiring:
 *   SDA -> D2 / GPIO4
 *   SCL -> D1 / GPIO5
 */

#include <Wire.h>

const uint8_t SDA_PIN = D2;
const uint8_t SCL_PIN = D1;

const uint8_t SENSOR_ADDRESS = 0x76;
const uint8_t CHIP_ID_REGISTER = 0xD0;

uint8_t readRegister(uint8_t reg)
{
    Wire.beginTransmission(SENSOR_ADDRESS);
    Wire.write(reg);
    Wire.endTransmission(false);

    Wire.requestFrom(SENSOR_ADDRESS, (uint8_t)1);

    if (Wire.available())
    {
        return Wire.read();
    }

    return 0x00;
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Wire.begin(SDA_PIN, SCL_PIN);

    Serial.println();
    Serial.println("================================");
    Serial.println(" Smart Greenhouse Controller");
    Serial.println(" Sensor Identification Test");
    Serial.println("================================");
    Serial.println();

    uint8_t chipID = readRegister(CHIP_ID_REGISTER);

    Serial.print("I2C Address : 0x");
    Serial.println(SENSOR_ADDRESS, HEX);

    Serial.print("Chip ID     : 0x");
    Serial.println(chipID, HEX);

    Serial.println();

    if (chipID == 0x60)
    {
        Serial.println("Sensor detected: BME280");
        Serial.println("Temperature : YES");
        Serial.println("Humidity    : YES");
        Serial.println("Pressure    : YES");
    }
    else if (chipID == 0x58)
    {
        Serial.println("Sensor detected: BMP280");
        Serial.println("Temperature : YES");
        Serial.println("Humidity    : NO");
        Serial.println("Pressure    : YES");
    }
    else
    {
        Serial.println("Unknown sensor ID.");
    }
}

void loop()
{
    // Identification only needs to run once.
}