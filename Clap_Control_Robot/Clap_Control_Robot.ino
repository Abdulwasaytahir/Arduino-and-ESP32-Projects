#include "driver/i2s.h"

// ================== MOTOR PINS ==================
#define IN1 5
#define IN2 18
#define IN3 19
#define IN4 21
#define ENA 17
#define ENB 16

// ================== I2S MIC PINS ==================
#define I2S_WS 25
#define I2S_SD 32
#define I2S_SCK 26

#define SAMPLE_RATE 44100
#define I2S_NUM I2S_NUM_0
#define BUFFER_LEN 1024

int threshold = 30; // scaled threshold
unsigned long lastSoundTime = 0;
int clapCount = 0;
unsigned long clapDelay = 400; // debounce between claps (ms)

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);

  // Motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  stopRobot();

  // I2S Config
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, 
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = BUFFER_LEN,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM, &pin_config);
}

// ================== LOOP ==================
void loop() {
  int soundAmp = readI2SAmplitude();
  int scaledAmp = soundAmp / 1000;

  Serial.println(scaledAmp);

  // Clap detection
  if (scaledAmp > threshold && millis() - lastSoundTime > clapDelay) {
    clapCount++;
    lastSoundTime = millis();
    Serial.print("Clap Detected! Count: "); Serial.println(clapCount);
  }

  // Execute command if 800ms passed since last clap
  if (clapCount > 0 && millis() - lastSoundTime > 800) {
    executeCommand(clapCount);
    clapCount = 0;
  }
}

// ================== COMMAND EXECUTION ==================
void executeCommand(int count) {
  if (count == 1) {
    moveForward();
  } else if (count == 2) {
    stopRobot();
  } else if (count == 3) {
    turnLeft();
  } else if (count == 4) {
    turnRight();
  } else {
    stopRobot();
  }
}

// ================== I2S READ FUNCTION ==================
int readI2SAmplitude() {
  int32_t buffer[BUFFER_LEN];
  size_t bytes_read;
  i2s_read(I2S_NUM, &buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);

  int32_t maxVal = 0;
  for (int i = 0; i < bytes_read/4; i++) {
    int32_t val = abs(buffer[i]);
    if (val > maxVal) maxVal = val;
  }

  return maxVal;
}

// ================== MOTOR FUNCTIONS ==================
void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 200);
  analogWrite(ENB, 200);
}

void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 180);
  analogWrite(ENB, 180);
}

void turnRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, 180);
  analogWrite(ENB, 180);
}

void stopRobot() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
