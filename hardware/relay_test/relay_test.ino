/*
 * Smart Greenhouse Controller
 * Relay Trigger Test
 *
 * Board:
 *   WeMos D1 R2 V2.1.0 / ESP8266
 *
 * Relay:
 *   ACEBOTT 5V Relay Module
 *   SONGLE SRD-05VDC-SL-C
 *
 * Wiring:
 *   Relay S -> D5 / GPIO14
 *   Relay V -> 5V
 *   Relay G -> GND
 *
 * Purpose:
 *   Determine the relay trigger logic and verify
 *   reliable ON/OFF operation.
 *
 * IMPORTANT:
 *   No mains-voltage load must be connected
 *   during this test.
 */

const uint8_t RELAY_PIN = D5;

void setup()
{
  Serial.begin(115200);
  delay(1000);

  pinMode(RELAY_PIN, OUTPUT);

  Serial.println();
  Serial.println("================================");
  Serial.println(" Smart Greenhouse Controller");
  Serial.println(" Relay Trigger Test");
  Serial.println("================================");
  Serial.println();

  Serial.println("Relay test starting.");
  Serial.println("No mains load should be connected.");
  Serial.println();
}

void loop()
{
  Serial.println("--------------------------------");
  Serial.println("GPIO14 / D5 -> LOW");
  digitalWrite(RELAY_PIN, LOW);

  delay(3000);

  Serial.println("GPIO14 / D5 -> HIGH");
  digitalWrite(RELAY_PIN, HIGH);

  delay(3000);
}