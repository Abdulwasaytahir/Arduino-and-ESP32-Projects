#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <driver/i2s.h>

// WiFi credentials
const char* ssid = "home";
const char* password = "6strings";
// Google Cloud Speech-to-Text API URL
const char* speechToTextApiUrl =  "https://speech.googleapis.com/v1/speech:recognize?key=AIzaSyAEU-HlZkR0eaJciJO7kOsP6RLTHNgGUD4";

// I2S configuration for the INMP441 microphone
#define I2S_WS 25
#define I2S_SD 32
#define I2S_SCK 33
#define I2S_PORT I2S_NUM_0

// Speaker settings (PWM pin for HW104 speaker)
#define SPEAKER_PIN 26

// Buffer for audio data
int16_t i2sBuffer[1024];

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize I2S for the microphone
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);

  // Set speaker pin as output pinMode(SPEAKER_PIN,OUTPUT);
}

void loop() {
  size_t bytesRead;

  // Capture audio data from the microphone
  i2s_read(I2S_PORT, (char*)i2sBuffer, sizeof(i2sBuffer), &bytesRead, portMAX_DELAY);

  // Send the audio data to Google Cloud for recognition
  if (bytesRead > 0) {
    sendAudioToSpeechToText(i2sBuffer, bytesRead);
  }

  delay(5000);  // Adjust delay for continuous listening
}

// Function to send audio data to Google Speech-to-Text API
void sendAudioToSpeechToText(int16_t* audioData, size_t length) {
  HTTPClient http;
  http.begin(speechToTextApiUrl);

  http.addHeader("Content-Type", "application/json");

  // Prepare JSON request body for the API
  String jsonRequest;
  jsonRequest = "{\"config\":{\"encoding\":\"LINEAR16\",\"sampleRateHertz\":16000,\"languageCode\":\"en-US\"},";
  jsonRequest += "\"audio\":{\"content\":\"" + base64Encode((byte*)audioData, length) + "\"}}";

  int httpResponseCode = http.POST(jsonRequest);

  // Handle the response from Google Cloud
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Server response: " + response);

    // Parse JSON and get the transcript
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, response);
    const char* transcript = doc["results"][0]["alternatives"][0]["transcript"];
    Serial.println("Transcription: " + String(transcript));

    // Respond to commands based on the transcription
    if (String(transcript) == "turn on speaker") {
      playSound();  // Function to play sound on HW104 speaker
    }
  } else {
    Serial.println("Error in sending POST request");
  }

  http.end();
}

// Function to encode audio data in base64
String base64Encode(byte* data, size_t length) {
  String encoded = "";
  const char base64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  for (size_t i = 0; i < length; i += 3) {
    uint32_t n = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
    encoded += base64Chars[(n >> 18) & 63];
    encoded += base64Chars[(n >> 12) & 63];
    encoded += base64Chars[(n >> 6) & 63];
    encoded += base64Chars[n & 63];
  }

  return encoded;
}

// Function to play sound through HW104 speaker
void playSound() {
  analogWrite(SPEAKER_PIN, 255);  // Play a sound by sending a PWM signal to the speaker
  delay(1000);  // Sound duration
  analogWrite(SPEAKER_PIN, 0);  // Stop the sound
}
