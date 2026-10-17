#include <Arduino.h>

#include "config.h"
#include "types.h"
#include "sensors.h"
#include "vpd.h"


void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);

    delay(500);

    Serial.println();
    Serial.println("================================");
    Serial.println(" Smart Greenhouse Controller");
    Serial.println(" VPD Module Integration Test");
    Serial.println("================================");

    if (!initializeSensors())
    {
        Serial.println("ERROR: Sensor initialization failed.");
        Serial.println("Check BME280 wiring and I2C address.");

        while (true)
        {
            delay(1000);
        }
    }

    Serial.println("Sensor initialization: PASS");
    Serial.println();
}


void loop()
{
    SensorData data = readSensors();

    float airVPD = NAN;

    if (data.environmentalSensorValid)
    {
        airVPD = calculateAirVPD(
            data.temperature,
            data.humidity
        );
    }

    Serial.println("--------------------------------");

    if (!data.environmentalSensorValid)
    {
        Serial.println("ERROR: Invalid environmental sensor data.");
    }
    else
    {
        Serial.print("Temperature : ");
        Serial.print(data.temperature, 2);
        Serial.println(" C");

        Serial.print("Humidity    : ");
        Serial.print(data.humidity, 2);
        Serial.println(" %");

        Serial.print("Pressure    : ");
        Serial.print(data.pressure, 2);
        Serial.println(" hPa");
    }

    Serial.print("Soil RAW    : ");
    Serial.println(data.soilRaw);

    Serial.print("Soil Moist. : ");
    Serial.print(data.soilPercent, 1);
    Serial.println(" %");

    if (!isnan(airVPD)){
        Serial.print("Air VPD      : ");
        Serial.print(airVPD, 2);
        Serial.println(" kPa");
    }
    else
    {
        Serial.println("Air VPD      : INVALID");
    }

    delay(SENSOR_READ_INTERVAL_MS);
}