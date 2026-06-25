#include <driver/i2s.h>

#define I2S_WS 25    // LRCLK / WS
#define I2S_SD 22    // Serial Out
#define I2S_SCK 26   // Bit Clock
#define SAMPLE_RATE 16000
#define I2S_NUM I2S_NUM_0

void setup() {
  Serial.begin(115200);

  // I2S config
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
      .communication_format = I2S_COMM_FORMAT_I2S_MSB,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 4,
      .dma_buf_len = 1024,
      .use_apll = false
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

void loop() {
  int32_t buffer[128];
  size_t bytesRead;
  i2s_read(I2S_NUM, &buffer, sizeof(buffer), &bytesRead, portMAX_DELAY);

  int32_t maxVal = 0;
  for(int i = 0; i < 128; i++){
    int32_t val = abs(buffer[i] >> 14); // Scale down for readability
    if(val > maxVal) maxVal = val;
  }

  Serial.println(maxVal); // Peak amplitude
  delay(50);
}
