#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"

#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

Audio audio;

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin("home", "6strings"); // Replace with your WiFi

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nWiFi Connected!");

  // I2S configuration
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(20);  // 0-100, safer to start lower

  // Play Google TTS
  audio.connecttospeech("Hello from Abdul Wasay Tahir", "en");
}

void loop() {
  audio.loop();
}

void audio_info(const char *info) {
  Serial.print("audio_info: "); 
  Serial.println(info);
}
