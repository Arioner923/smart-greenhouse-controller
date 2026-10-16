/*
 * Smart Greenhouse Controller
 * Soil Moisture Sensor Calibration Test
 *
 * Board:
 *   WeMos D1 R2 V2.1.0 / ESP8266
 *
 * Sensor:
 *   Capacitive Soil Moisture Sensor
 *
 * Wiring:
 *   Sensor VCC  -> 3.3V
 *   Sensor GND  -> GND
 *   Sensor AOUT -> A0
 *
 * Purpose:
 *   Collect multiple ADC samples and calculate:
 *     - Average value
 *     - Minimum value
 *     - Maximum value
 *     - Measurement spread
 *
 * These results will be used to determine:
 *   DRY_REFERENCE
 *   WET_REFERENCE
 *
 * Important:
 *   No soil moisture percentage is calculated yet.
 *   Percentage conversion will be added only after
 *   the dry and wet calibration values are verified.
 */

const uint8_t SOIL_PIN = A0;

// Number of ADC samples used for each measurement block.
const uint8_t SAMPLE_COUNT = 30;

// Delay between individual ADC samples.
const unsigned long SAMPLE_INTERVAL_MS = 50;

// Delay between complete measurement reports.
const unsigned long REPORT_INTERVAL_MS = 3000;


/*
 * Reads multiple ADC samples from the soil sensor
 * and reports basic statistics.
 */
void readSoilCalibrationBlock()
{
  long sum = 0;

  int minimumValue = 1023;
  int maximumValue = 0;

  for (uint8_t i = 0; i < SAMPLE_COUNT; i++)
  {
    int value = analogRead(SOIL_PIN);

    sum += value;

    if (value < minimumValue)
    {
      minimumValue = value;
    }

    if (value > maximumValue)
    {
      maximumValue = value;
    }

    delay(SAMPLE_INTERVAL_MS);
  }

  float averageValue = (float)sum / SAMPLE_COUNT;

  int spread = maximumValue - minimumValue;

  Serial.println("--------------------------------");

  Serial.print("Samples : ");
  Serial.println(SAMPLE_COUNT);

  Serial.print("Average : ");
  Serial.println(averageValue, 1);

  Serial.print("Minimum : ");
  Serial.println(minimumValue);

  Serial.print("Maximum : ");
  Serial.println(maximumValue);

  Serial.print("Spread  : ");
  Serial.println(spread);

  Serial.println("--------------------------------");
  Serial.println();
}


void setup()
{
  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println(" Smart Greenhouse Controller");
  Serial.println(" Soil Sensor Calibration Test");
  Serial.println("================================");
  Serial.println();

  Serial.println("Calibration mode started.");
  Serial.println();
  Serial.println("Record measurements for:");
  Serial.println("1. DRY SOIL");
  Serial.println("2. WET SOIL");
  Serial.println();
}


void loop()
{
  readSoilCalibrationBlock();

  delay(REPORT_INTERVAL_MS);
}