// ESP32 Devkit V1 Serial Monitor Test

void setup() {
  // Serial communication start ki 115200 baud rate par
  Serial.begin(115200);
  delay(1000); 
  Serial.println("Hello ESP32 - Board is working fine!");
}

void loop() {
  Serial.println("ESP32 is running...");
  delay(2000);  // 2 seconds baad phir message repeat hoga
}

