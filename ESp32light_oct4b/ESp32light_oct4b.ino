// ESP32 Devkit V1 Basic Blink Test

#define LED_BUILTIN 2   // ESP32 Devkit V1 mai builtin LED GPIO 2 par hoti hai

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // LED pin ko output banaya
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH); // LED ON
  delay(1000);                     // 1 second rukna
  digitalWrite(LED_BUILTIN, LOW);  // LED OFF
  delay(1000);                     // 1 second rukna
}
