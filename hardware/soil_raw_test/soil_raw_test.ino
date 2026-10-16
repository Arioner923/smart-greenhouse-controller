/*
 * Smart Greenhouse Controller
 * Capacitive Soil Moisture Sensor - Raw ADC Test
 *
 * Board:
 *   WeMos D1 R2 V2.1.0 / ESP8266
 *
 * Wiring:
 *   Sensor VCC  -> 3.3V
 *   Sensor GND  -> GND
 *   Sensor AOUT -> A0
 *
 * Purpose:
 *   Read and validate the raw analog output of the
 *   capacitive soil moisture sensor.
 *
 * Note:
 *   No percentage conversion is performed yet.
 *   Calibration will be completed in a later test.
 */

const uint8_t SOIL_PIN = A0;

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println(" Smart Greenhouse Controller");
  Serial.println(" Soil Moisture Raw ADC Test");
  Serial.println("================================");
  Serial.println();
}

void loop()
{
  int soilRaw = analogRead(SOIL_PIN);

  Serial.print("Soil RAW : ");
  Serial.println(soilRaw);

  delay(1000);
}