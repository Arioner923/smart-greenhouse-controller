/*
 * Smart Greenhouse Controller
 * VPD Calculation Test
 *
 * Board:
 *   WeMos D1 R2 V2.1.0 / ESP8266
 *
 * Connected devices:
 *   - BME280
 *   - Capacitive soil moisture sensor
 *   - ACEBOTT 5V relay module
 *
 * Purpose:
 *   Read environmental data and calculate
 *   Air Vapor Pressure Deficit (VPD).
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <math.h>

const uint8_t SDA_PIN = D2;
const uint8_t SCL_PIN = D1;
const uint8_t RELAY_PIN = D5;
const uint8_t SOIL_PIN = A0;
const uint8_t BME_ADDRESS = 0x76;

// Soil calibration references
const int SOIL_AIR_REFERENCE = 746;
const int SOIL_WATER_REFERENCE = 390;

Adafruit_BME280 bme;

float calculateSoilPercent(int rawValue)
{
  float percent =
      ((float)(SOIL_AIR_REFERENCE - rawValue) /
       (SOIL_AIR_REFERENCE - SOIL_WATER_REFERENCE)) * 100.0;

  if (percent < 0.0)
  {
    percent = 0.0;
  }

  if (percent > 100.0)
  {
    percent = 100.0;
  }

  return percent;
}

/*
 * Calculates saturation vapor pressure (SVP)
 * in kPa using air temperature in degrees Celsius.
 */
float calculateSVP(float temperatureC)
{
  return 0.6108 *
         exp((17.27 * temperatureC) /
             (temperatureC + 237.3));
}

/*
 * Calculates Air VPD in kPa.
 */
float calculateVPD(float temperatureC, float relativeHumidity)
{
  float svp = calculateSVP(temperatureC);

  return svp *
         (1.0 - relativeHumidity / 100.0);
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Wire.begin(SDA_PIN, SCL_PIN);

  pinMode(RELAY_PIN, OUTPUT);

  // Relay is active HIGH.
  // Keep it OFF during this validation test.
  digitalWrite(RELAY_PIN, LOW);

  Serial.println();
  Serial.println("================================");
  Serial.println(" Smart Greenhouse Controller");
  Serial.println(" VPD Calculation Test");
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
  Serial.println("Relay initialized: OFF");
  Serial.println();
}

void loop()
{
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;

  int soilRaw = analogRead(SOIL_PIN);
  float soilPercent = calculateSoilPercent(soilRaw);

  float vpd = calculateVPD(temperature, humidity);

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

  Serial.print("VPD         : ");
  Serial.print(vpd, 2);
  Serial.println(" kPa");

  Serial.print("Soil RAW    : ");
  Serial.println(soilRaw);

  Serial.print("Soil Moist. : ");
  Serial.print(soilPercent, 1);
  Serial.println(" %");

  Serial.println("Relay       : OFF");

  Serial.println("--------------------------------");
  Serial.println();

  delay(2000);
}