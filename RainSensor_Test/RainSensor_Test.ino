#define RAIN_SENSOR_PIN 23  // Change to the GPIO pin you used

void setup() {
  Serial.begin(115200);      // Start the Serial communication
  pinMode(RAIN_SENSOR_PIN, INPUT);  // Set the rain sensor pin as input
}

void loop() {
  int sensorValue = digitalRead(RAIN_SENSOR_PIN);  // Read the sensor value

  if (sensorValue == HIGH) {
    Serial.println("No Rain");
  } else {
    Serial.println("It's Raining!");
  }
  
  delay(1000);  // Wait for a second before the next reading
}