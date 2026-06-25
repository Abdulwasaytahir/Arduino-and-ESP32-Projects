#define LDR_PIN 34  // Analog pin connected to LDR

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);  // Set ADC resolution (0-4095)
}

void loop() {
  int lightLevel = analogRead(LDR_PIN);  // Read analog value from LDR
  Serial.print("Light Level: ");
  Serial.println(lightLevel);
  delay(1000);  // Delay for 1 second
}