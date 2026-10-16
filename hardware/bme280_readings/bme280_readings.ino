/*
 * Smart Greenhouse Controller
 * BME280 Environmental Sensor Test
 *
 * Board:
 *   WeMos D1 R2 V2.1.0 / ESP8266
 *
 * Sensor:
 *   BME280
 *
 * Verified I2C address:
 *   0x76
 *
 * Wiring:
 *   SDA -> D2 / GPIO4
 *   SCL -> D1 / GPIO5
 *   VCC -> 3.3V
 *   GND -> GND
 *
 * Purpose:
 *   Validate temperature, humidity
 *   and atmospheric pressure readings.
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

const uint8_t SDA_PIN = D2;
const uint8_t SCL_PIN = D1;
const uint8_t BME_ADDRESS = 0x76;

Adafruit_BME280 bme;

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println();
  Serial.println("================================");
  Serial.println(" Smart Greenhouse Controller");
  Serial.println(" BME280 Environmental Test");
  Serial.println("================================");
  Serial.println();

  if (!bme.begin(BME_ADDRESS, &Wire))
  {
    Serial.println("ERROR: BME280 not detected.");

    while (true)
    {
      delay(1000);
    }
  }

  Serial.println("BME280 detected successfully.");
  Serial.println("I2C Address: 0x76");
  Serial.println();
}

void loop()
{
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;

  Serial.println("--------------------------------");

  Serial.print("Temperature : ");
  Serial.print(temperature, 2);
  Serial.println(" C");

  Serial.print("Humidity    : ");
  Serial.print(humidity, 2);
  Serial.println(" %");

  Serial.print("Pressure    : ");
  Serial.print(pressure, 2);
  Serial.println(" hPa");

  Serial.println("--------------------------------");
  Serial.println();

  delay(2000);
}