#include <WiFi.h>
#include <AsyncUDP.h>
#include <driver/i2s.h>

// WiFi credentials
const char* ssid = "your-SSID";
const char* password = "your-PASSWORD";

// UDP settings
AsyncUDP udp;
const int udpPort = 1234;

// I2S setup
#define I2S_WS 25
#define I2S_SD 32
#define I2S_SCK 33
#define I2S_PORT I2S_NUM_0

// Define whether this is the Microphone (Sender) or Speaker (Receiver)
#define IS_SENDER true  // Set to 'false' for the Speaker (Receiver)

void setup() {
  Serial.begin(115200);

  // Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // I2S configuration
  i2s_config_t i2s_config = {
    .mode = IS_SENDER ? (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX) : (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = IS_SENDER ? I2S_PIN_NO_CHANGE : I2S_SD,
    .data_in_num = IS_SENDER ? I2S_SD : I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);

  // Initialize UDP
  if (IS_SENDER) {
    if (udp.connect(IPAddress(192, 168, 4, 2), udpPort)) {  // Change to receiver's IP
      Serial.println("UDP connected (Sender)");
    }
  } else {
    if (udp.listen(udpPort)) {
      udp.onPacket([](AsyncUDPPacket packet) {
        // Received audio data
        int16_t* audioData = (int16_t*)packet.data();
        size_t len = packet.length();

        // Play the received audio
        size_t bytesWritten;
        i2s_write(I2S_PORT, audioData, len, &bytesWritten, portMAX_DELAY);
      });
      Serial.println("Listening for UDP packets (Receiver)");
    }
  }
}

void loop() {
  if (IS_SENDER) {
    // Buffer to hold audio data
    int16_t i2sBuffer[256];
    size_t bytesRead;

    // Read audio data from microphone
    i2s_read(I2S_PORT, i2sBuffer, sizeof(i2sBuffer), &bytesRead, portMAX_DELAY);

    // Send audio data over UDP
    udp.writeTo((uint8_t*)i2sBuffer, bytesRead, IPAddress(192, 168, 4, 2), udpPort);  // Change to receiver's IP
  }

  // Receiver code is handled in the UDP callback, so nothing is needed here
}
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <driver/i2s.h>

#define NUM_LEDS 1
#define DATA_PIN 14
#define BUTTON_PIN 27
#define IR_PIN 12

CRGB leds[NUM_LEDS];

// Network credentials
const char* ssid = "your-SSID";
const char* password = "your-PASSWORD";

// I2S configuration for INMP441 microphone
#define I2S_WS 25
#define I2S_SD 32
#define I2S_SCK 33
#define I2S_PORT I2S_NUM_0

AsyncWebServer server(80);

// Function to set LED color
void setLEDColor(CRGB color) {
    leds[0] = color;
    FastLED.show();
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  // Wait for Wi-Fi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  setLEDColor(CRGB::Green);  // Power status

  // I2S configuration for INMP441 microphone
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = 0,  // Default interrupt priority
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);

  // Web server endpoint
  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    String message;
    if (request->hasParam("speech", true)) {
        message = request->getParam("speech", true)->value();
        Serial.println("Speech received: " + message);

        setLEDColor(CRGB::Blue); // Command processing

        // Process command
        processCommand(message);

        setLEDColor(CRGB::Green); // Command processed
    } else {
        message = "No speech data received";
    }
    request->send(200, "text/plain", "Command processed: " + message);
  });

  server.begin();
}

void loop() {
  // Handle obstacle detection
  if (digitalRead(IR_PIN) == LOW) {
    setLEDColor(CRGB::Red); // Error/Obstacle detected
  }
}

void processCommand(String command) {
  // Process the voice command here
  if (command == "turn on light") {
    Serial.println("Turning on light...");
    // Perform action to turn on light
  }
}