#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <I2S.h>
#include <FastLED.h>

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
  I2S.setPins(I2S_WS, I2S_SCK, I2S_SD);
  I2S.begin(I2S_PHILIPS_MODE, 44100, 32);

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request){
    String message;
    if (request->hasParam("speech", true)) {
        message = request->getParam("speech", true)->value();
        Serial.println("Speech received: " + message);
        
        setLEDColor(CRGB::Blue); // Command processing
        
        // Process command and send data to the slave module
        // Example: Turning on a device or other action
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
  // Handle incoming speech data, sensor inputs, etc.
  if (digitalRead(IR_PIN) == LOW) {
    setLEDColor(CRGB::Red); // Error/Obstacle detected
  }
}

void processCommand(String command) {
  // Process the voice command here and send it to the slave module
  if (command == "turn on light") {
    // Example action
    // You would communicate with the slave module here
    Serial.println("Turning on light...");
  }
}
#include <WiFi.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <driver/i2s.h>

#define NUM_LEDS 1
#define DATA_PIN 14

CRGB leds[NUM_LEDS];

// I2S configuration for speaker
#define I2S_WS 25
#define I2S_SD 32
#define I2S_SCK 33
#define I2S_PORT I2S_NUM_0

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  setLEDColor(CRGB::Green);  // Power status

  // I2S configuration for audio output
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_SD,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
}

void loop() {
  // Check for commands from the master module and execute them
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    Serial.println("Command received: " + command);
    
    setLEDColor(CRGB::Blue); // Command processing
    
    if (command == "turn on light") {
      // Turn on a light or perform an action
      Serial.println("Light turned on.");
    }
    
    setLEDColor(CRGB::Green); // Command processed
  }
}

void setLEDColor(CRGB color) {
  leds[0] = color;
  FastLED.show();
}
