#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <driver/i2s.h>

// WiFi credentials
const char* ssid = "home";
const char* password = "6strings";

// Google Cloud Speech-to-Text API URL (replace with your endpoint if different)
const char* speechToTextApiUrl = "https://speech.googleapis.com/v1/speech:recognize?key=YOUR_API_KEY";

// I2S configuration for the INMP441 microphone
#define I2S_WS 25
#define I2S_SD 32
#define I2S_SCK 33
#define I2S_PORT I2S_NUM_0

// Buffer to hold audio data
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

  // Configure I2S
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
}

void loop() {
  size_t bytesRead;

  // Capture audio data from the microphone
  i2s_read(I2S_PORT, (char*)i2sBuffer, sizeof(i2sBuffer), &bytesRead, portMAX_DELAY);

  // Send the audio data to the cloud for speech recognition
  if (bytesRead > 0) {
    sendAudioToSpeechToText(i2sBuffer, bytesRead);
  }

  delay(1000); // Adjust as per your application
}

void sendAudioToSpeechToText(int16_t* audioData, size_t length) {
  HTTPClient http;
  http.begin(speechToTextApiUrl);

  http.addHeader("Content-Type", "application/json");

  // Prepare the JSON request body for the API
  String jsonRequest;
  jsonRequest = "{\"config\":{\"encoding\":\"LINEAR16\",\"sampleRateHertz\":16000,\"languageCode\":\"en-US\"},";
  jsonRequest += "\"audio\":{\"content\":\"" + base64Encode((byte*)audioData, length) + "\"}}";

  int httpResponseCode = http.POST(jsonRequest);

  // Handle the response from the server
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Server response: " + response);

    // Parse the JSON response and extract the transcription
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, response);
    const char* transcript = doc["results"][0]["alternatives"][0]["transcript"];
    Serial.println("Transcription: " + String(transcript));
  } else {
    Serial.println("Error in sending POST request");
  }

  http.end();
}

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
