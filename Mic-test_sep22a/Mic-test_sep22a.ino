#include <driver/i2s.h>
// I2S configuration
const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
};

const i2s_pin_config_t pin_config = {
    .bck_io_num = 26,   // SCK
    .ws_io_num = 25,    // LRCK
    .data_in_num = 33   // SD
};

void setup() {
    Serial.begin(115200);

    // Install and start I2S driver
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_set_clk(I2S_NUM_0, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

void loop() {
    int16_t audio_buffer[64];
    size_t bytes_read;
    i2s_read(I2S_NUM_0, &audio_buffer, sizeof(audio_buffer), &bytes_read, portMAX_DELAY);
    if (bytes_read > 0) {
        Serial.println(audio_buffer[0]); 
        DEC;
       // Print one audio sample for testing
}
}