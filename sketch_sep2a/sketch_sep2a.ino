#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>
#include "I2S.h"

#define USE_SERIAL Serial
#define LED_PIN 2
#define BUTTON_PIN 23

const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";
const char* speechToTextAPIKey = "Your_Speech_to_Text_API_Key";
const char* gpt3ApiKey = "Your_GPT3_API_Key";
const char* speechToTextURL = "---";
const char* gpt3URL = "https://api.openai.com/v1/completions";

// I2S setup
I2S i2s;
#define SAMPLE_RATE (16000)

void setup() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi");

    i2s.begin(I2S_PHILIPS_MODE, SAMPLE_RATE, 32);
}

void loop() {
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println("Button pressed, starting recording...");
        digitalWrite(LED_PIN, HIGH);
        
        // Record audio
        const int seconds = 5;
        int bufferSize = SAMPLE_RATE * seconds * 4;
        int16_t *buffer = (int16_t *)malloc(bufferSize);
        i2s.read(buffer, bufferSize);

        Serial.println("Audio recorded, sending to Speech-to-Text API...");
        String transcript = transcribeAudio(buffer, bufferSize);
        Serial.print("Transcript: ");
        Serial.println(transcript);

        Serial.println("Generating response with GPT-3...");
        String response = generateResponse(transcript);
        Serial.print("GPT-3 Response: ");
        Serial.println(response);

        digitalWrite(LED_PIN, LOW);
        free(buffer);
        delay(5000);  // Delay to avoid rapid re-triggering
    }
}

String transcribeAudio(int16_t *audioData, int dataSize) {
    HTTPClient http;
    String url = String(speechToTextAPIKey) + speechToTextURL;
    http.begin(url);

    String payload = "{\"config\":{\"encoding\":\"LINEAR16\",\"sampleRateHertz\":16000,\"languageCode\":\"en-US\"},\"audio\":{\"content\":\"";
    String audioBase64 = base64::encode((byte*)audioData, dataSize);
    payload += audioBase64;
    payload += "\"}}";

    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode == HTTP_CODE_OK) {
        String response = http.getString();
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, response);
        return doc["results"][0]["alternatives"][0]["transcript"].as<String>();
    } else {
        Serial.printf("Error: %s\n", http.errorToString(httpResponseCode).c_str());
        return "";
    }
}

String generateResponse(const String& transcript) {
    HTTPClient http;
    http.begin(gpt3URL);

    String payload = "{\"model\": \"text-davinci-003\", \"prompt\": \"" + transcript + "\", \"temperature\": 0.7, \"max_tokens\": 150}";

    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + String(gpt3ApiKey));

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode == HTTP_CODE_OK) {
        String response = http.getString();
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, response);
        return doc["choices"][0]["text"].as<String>();
    } else {
        Serial.printf("Error: %s\n", http.errorToString(httpResponseCode).c_str());
        return "";
    }
}
