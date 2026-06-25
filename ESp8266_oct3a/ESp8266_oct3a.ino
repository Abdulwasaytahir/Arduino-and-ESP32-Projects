#include <Wire.h>
#include <ESP8266WiFi.h>
#include <I2S.h>

#define HW104_PIN 5  // Pin connected to HW104 sound sensor OUT
#define SAMPLE_RATE 16000

const char* ssid = "home";
const char* password = "6strings";

// I2S Setup
I2S i2s;

// Variables to store sound data
const int bufferSize = SAMPLE_RATE * 5;  // 5 seconds buffer
int16_t audioBuffer[bufferSize];

void setup() {
  pinMode(HW104_PIN, INPUT);  // Set the sound sensor pin as input
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Setup I2S for INMP441 microphone
  i2s.begin(I2S_PHILIPS_MODE, SAMPLE_RATE, 16);
}

void loop() {
  if (digitalRead(HW104_PIN) == HIGH) {  // Sound detected by HW104
    Serial.println("Sound detected, recording...");
    recordAudio();
  }
}

void recordAudio() {
  for (int i = 0; i < bufferSize; i++) {
    i2s.read(&audioBuffer[i], sizeof(int16_t));  // Capture audio samples
  }
  Serial.println("Recording complete. Audio data available for further processing.");
  // Process or transmit audio data here
}