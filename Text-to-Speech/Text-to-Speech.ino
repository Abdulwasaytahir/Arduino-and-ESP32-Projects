#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Audio.h"

const char* ssid = "home";
const char* password = "6strings";
const char* chatgpt_token = "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";
const char* temperature = "0";
const char* max_tokens = "50";  // Increase if you want longer answers

String Question = "";

#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26

#define TTS_CHUNK_SIZE 16384  // 16 KB per chunk to prevent OOM

Audio audio;

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // wait for WiFi connection
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    audio.loop(); // keep audio loop running
  }
  Serial.println("\nConnected");
  Serial.print("IP address: "); Serial.println(WiFi.localIP());

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(100);
}

// Function to play TTS in memory-friendly chunks
void playTTS(const char* text) {
  int len = strlen(text);
  for (int i = 0; i < len; i += TTS_CHUNK_SIZE) {
    int chunkLen = min(TTS_CHUNK_SIZE, len - i);
    char chunk[TTS_CHUNK_SIZE + 1]; // +1 for null terminator
    strncpy(chunk, &text[i], chunkLen);
    chunk[chunkLen] = '\0';
    audio.connecttospeech(chunk, "en");
  }
}

void loop() {
  Serial.print("Ask your Question: ");

  // Collect Serial input safely
  while (!Serial.available()) {
    audio.loop();
  }

  Question = "";
  while (Serial.available()) {
    char c = Serial.read();
    if (c != '\n' && c != '\r') { // ignore newlines
      Question += c;
    }
    delay(1);
  }

  if (Question.length() == 0) return; // no input
  Serial.println("Question: " + Question);

  // Prepare HTTPS request
  HTTPClient https;
  if (https.begin("https://api.openai.com/v1/completions")) {
    https.addHeader("Content-Type", "application/json");
    https.addHeader("Authorization", String("Bearer ") + chatgpt_token);

    String payload = String("{\"model\":\"text-davinci-003\",\"prompt\":\"") + Question + 
                     String("\",\"temperature\":") + temperature + 
                     String(",\"max_tokens\":") + max_tokens + String("}");

    int httpCode = https.POST(payload);

    if (httpCode == HTTP_CODE_OK) {
      String response = https.getString();
      DynamicJsonDocument doc(4096); // handle longer responses
      DeserializationError error = deserializeJson(doc, response);

      if (!error) {
        String Answer = doc["choices"][0]["text"].as<String>();
        Answer.trim(); // remove leading/trailing spaces/newlines
        Serial.println("Answer: " + Answer);

        // Play TTS in memory-safe chunks
        playTTS(Answer.c_str());
      } else {
        Serial.println("JSON parse error: " + String(error.c_str()));
      }
    } else {
      Serial.printf("HTTPS POST failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();
  } else {
    Serial.println("Unable to connect to server");
  }

  Question = ""; // clear for next loop
}

void audio_info(const char *info) {
  Serial.print("audio_info: "); Serial.println(info);
}
