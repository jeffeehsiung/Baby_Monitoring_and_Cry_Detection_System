#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "espnow_mic.h"
#include "sd_record.h"
#include "audio_example_file.h"

#if (!CONFIG_IDF_TARGET_ESP32)
#include "i2s_recv_std_config.h"
#endif

static const char* TAG = "espnow_mic";
StreamBufferHandle_t spk_stream_buf;
StreamBufferHandle_t fft_stream_buf;
StreamBufferHandle_t record_stream_buf;

uint8_t* mic_read_buf;
uint8_t* spk_write_buf;


// reference: https://www.codeinsideout.com/blog/freertos/notification/#two-looping-tasks
volatile unsigned int play_music = 0;
TaskHandle_t adcTaskHandle = NULL;
TaskHandle_t musicTaskHandle = NULL;
TaskHandle_t fftTaskHandle = NULL;

// i2s adc capture task
void i2s_adc_capture_task(void* task_param)
{
    // get the stream buffer handle from the task parameter
    StreamBufferHandle_t mic_stream_buf = (StreamBufferHandle_t) task_param;

    mic_read_buf = calloc(READ_BUF_SIZE_BYTES,sizeof(char));

    // enable i2s adc
    size_t bytes_read = 0; // to count the number of bytes read from the i2s adc
    TickType_t ticks_to_wait = 100; // wait 100 ticks for the mic_stream_buf to be available


    i2s_adc_enable(EXAMPLE_I2S_NUM);


    while(true){

        // read from i2s bus and use errno to check if i2s_read is successful
        if (i2s_read(EXAMPLE_I2S_NUM, (char*)mic_read_buf, READ_BUF_SIZE_BYTES, &bytes_read, ticks_to_wait) != ESP_OK) {
            ESP_LOGE(TAG, "Error reading from i2s adc: %d", errno);
            deinit_config();
            exit(errno);
        }
        // check if the number of bytes read is equal to the number of bytes to read
        if (bytes_read != READ_BUF_SIZE_BYTES) {
            ESP_LOGE(TAG, "Error reading from i2s adc: %d", errno);
            deinit_config();
            exit(errno);
        }

        /**
         * xstreambuffersend to fft task
        */
        #if FFT_TASK
        size_t byte_sent = xStreamBufferSend(fft_stream_buf,(void*) mic_read_buf, EXAMPLE_I2S_READ_LEN/16, portMAX_DELAY);
        if (byte_sent != (EXAMPLE_I2S_READ_LEN/16)) {
            ESP_LOGE(TAG, "Error: only sent %d bytes to the stream buffer out of %d \n", byte_sent, (EXAMPLE_I2S_READ_LEN/16));
            deinit_config();
            exit(errno);
        }
        #endif

        // scale the data to 8 bit
        i2s_adc_data_scale(mic_read_buf, mic_read_buf, READ_BUF_SIZE_BYTES);

        /**
         * xstreambuffersend is a blocking function that sends data to the stream buffer,
         * esp_now_send needs to send 128 packets of 250 bytes each, so the stream buffer needs to be able to hold at least 2-3 times of 128 * 250 bytes = BYTE_RATE bytes
         * */ 
        size_t espnow_byte = xStreamBufferSend(mic_stream_buf,(void*) mic_read_buf, READ_BUF_SIZE_BYTES, portMAX_DELAY);
        if (espnow_byte != READ_BUF_SIZE_BYTES) {
            ESP_LOGE(TAG, "Error: only sent %d bytes to the stream buffer out of %d \n", espnow_byte, READ_BUF_SIZE_BYTES);
        }

        /**
         * xstreambuffersend to sd record task
        */
        #if RECORD_TASK
        size_t record_byte = xStreamBufferSend(record_stream_buf,(void*) mic_read_buf, READ_BUF_SIZE_BYTES, portMAX_DELAY);
        // check if bytes sent is equal to bytes read
        if (record_byte != READ_BUF_SIZE_BYTES) {
            ESP_LOGE(TAG, "Error: only sent %d bytes to the stream buffer out of %d \n", record_byte, READ_BUF_SIZE_BYTES);
        }
        #endif

        // if play_music is set to high, signal the music task to play music
        if (play_music) {
            // // disable i2s adc
            // i2s_adc_disable(EXAMPLE_I2S_NUM);
            // signal the music task to play music
            xTaskGenericNotify(
                musicTaskHandle /* xTaskToNotify */,
                0 /* uxIndexToNotify */,
                0 /* ulValue */,
                eNoAction /* eAction */,
                NULL /* pulPreviousNotificationValue */
            );
            // disable i2s adc
            i2s_adc_disable(EXAMPLE_I2S_NUM);
            ESP_LOGE(TAG, "play_music is set to high, signal the music task to play music");
            // suspend itself
            vTaskSuspend(NULL);
        }
    }
    free(mic_read_buf);
    vTaskDelete(NULL);
    
}

/**
 * @brief Scale data to 8bit for data from ADC.
 *        DAC can only output 8 bit data.
 *        Scale each 16bit-wide ADC data to 8bit DAC data.
 */
void i2s_adc_data_scale(uint8_t * des_buff, uint8_t* src_buff, uint32_t len)
{
    uint32_t j = 0;
    uint32_t dac_value = 0;
    for (int i = 0; i < len; i += 2) {
        dac_value = ((((uint16_t) (src_buff[i + 1] & 0xf) << 8) | ((src_buff[i + 0]))));
        des_buff[j++] = 0;
        des_buff[j++] = dac_value * 256 / 4096;
    }
}



// i2s dac playback task
void i2s_dac_playback_task(void* task_param) {
    // get the stream buffer handle from the task parameter
    spk_stream_buf = (StreamBufferHandle_t)task_param;

    int intialized = 1;

    size_t bytes_written = 0;
    spk_write_buf = (uint8_t*) calloc(EXAMPLE_I2S_SAMPLE_RATE/2,sizeof(char));
    assert(spk_write_buf != NULL);

    while (true) {
        // read from the stream buffer, use errno to check if xstreambufferreceive is successful
        size_t num_bytes = xStreamBufferReceive(spk_stream_buf, (void*) spk_write_buf, EXAMPLE_I2S_SAMPLE_RATE/2, portMAX_DELAY);
        if (num_bytes > 0) {
            // send data to i2s dac
            esp_err_t err = i2s_write(EXAMPLE_I2S_NUM, spk_write_buf, num_bytes, &bytes_written, portMAX_DELAY);
            if ((err != ESP_OK) & (intialized == 0)) {
                printf("Error writing I2S: %0x\n", err);
                deinit_config();
                exit(err);
            }
            // reference: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2s.html
            // reference: https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/api-reference/peripherals/i2s.html#_CPPv49i2s_write10i2s_port_tPKv6size_tP6size_t10TickType_t
            // reference: i2s_write(I2S_NUM, samples_data, ((bits+8)/16)*SAMPLE_PER_CYCLE*4, &i2s_bytes_write, 100); 
            // this number is  without adc to dac scaling that is done in the i2s_adc_data_scale function, the i2s_write function needs to be called with the above parameters
        }
        else if(num_bytes != EXAMPLE_I2S_SAMPLE_RATE) {
            printf("Error: partial reading from net stream: %d\n", errno);
            deinit_config();
            exit(errno);
        }
        intialized = 0;
        
        #if EXAMPLE_I2S_BUF_DEBUG
            mic_disp_buf ((uint8_t*)spk_write_buf, EXAMPLE_I2S_READ_LEN);
        #endif
    }
    free(spk_write_buf);
    vTaskDelete(NULL);
}


/* call the init_auidio function for starting adc and filling the buf -second */
esp_err_t init_audio_trans(StreamBufferHandle_t mic_stream_buf, StreamBufferHandle_t fft_audio_buf, StreamBufferHandle_t record_audio_buf){ 
    printf("initializing i2s mic\n");
    fft_stream_buf = fft_audio_buf;
    record_stream_buf = record_audio_buf;

    /* thread for adc and filling the buf for the transmitter */
    xTaskCreate(i2s_adc_capture_task, "i2s_adc_capture_task", 4096, (void*) mic_stream_buf, 4, &adcTaskHandle); 

    return ESP_OK;
}

/* call the init_auidio function for starting adc and filling the buf -second */
esp_err_t init_audio_recv(StreamBufferHandle_t network_stream_buf){ 
    printf("initializing i2s spk\n");
    // /* thread for filling the buf for the reciever and dac */
#if CONFIG_IDF_TARGET_ESP32
    xTaskCreate(i2s_dac_playback_task, "i2s_dac_playback_task", 4096, (void*) network_stream_buf, 4, NULL);
#else
    xTaskCreate(i2s_std_playback_task, "i2s_std_playback_task", 4096,(void*) network_stream_buf, 4, NULL);
#endif
    return ESP_OK;
}

// /** debug functions below */

/**
 * @brief debug buffer data
 */
void mic_disp_buf(uint8_t* buf, int length)
{
#if EXAMPLE_I2S_BUF_DEBUG
    printf("\n=== MIC ===\n");
    for (int i = 0; i < length; i++) {
        printf("%02x ", buf[i]);
        if ((i + 1) % 8 == 0) {
            printf("\n");
        }
    }
    printf("\n=== MIC ===\n");
#endif
}

// music initialization
esp_err_t init_music(void* arg)
{
    // get the fft task handle from the task parameter
    fftTaskHandle = (TaskHandle_t) arg;
    // set the log level for the i2s driver
    esp_log_level_set("I2S", ESP_LOG_INFO);
    // create task for music_task function and pass the fft task handle as a parameter
    xTaskCreate(music_task, "music_task", 4096, (void*) fftTaskHandle, 5, &musicTaskHandle);
    return ESP_OK;
}


/**
 * @brief Reset i2s clock and mode
 */
void example_reset_play_mode(void)
{
    i2s_set_clk(EXAMPLE_I2S_NUM, EXAMPLE_I2S_SAMPLE_RATE, EXAMPLE_I2S_SAMPLE_BITS, EXAMPLE_I2S_CHANNEL_NUM);
}

/**
 * @brief Set i2s clock for example audio file
 */
void example_set_file_play_mode(void)
{
    ESP_LOGE(TAG, "Set i2s clock for example audio file");
    i2s_set_clk(EXAMPLE_I2S_NUM, EXAMPLE_I2S_SAMPLE_RATE, EXAMPLE_I2S_SAMPLE_BITS, 1); // channel 1 represent i2s_channel_t I2S_CHANNEL_MONO
}

/**
 * @brief Scale data to 16bit/32bit for I2S DMA output.
 *        DAC can only output 8bit data value.
 *        I2S DMA will still send 16 bit or 32bit data, the highest 8bit contains DAC data.
 */
int example_i2s_dac_data_scale(uint8_t* d_buff, uint8_t* s_buff, uint32_t len)
{
    uint32_t j = 0;
#if (EXAMPLE_I2S_SAMPLE_BITS == 16)
    for (int i = 0; i < len; i++) {
        d_buff[j++] = 0;
        d_buff[j++] = s_buff[i];
    }
    return (len * 2);
#else
    for (int i = 0; i < len; i++) {
        d_buff[j++] = 0;
        d_buff[j++] = 0;
        d_buff[j++] = 0;
        d_buff[j++] = s_buff[i];
    }
    return (len * 4);
#endif
}

/**
 * @brief I2S ADC/DAC example
 *        1. Play an example audio file(file format: 8bit/8khz/single channel)
 */
/* A task that blocks waiting to be notified and processes data */
void music_task(void*arg)
{
    int i2s_read_len = EXAMPLE_I2S_READ_LEN;
    size_t bytes_written;
    // get the fft task handle from the task parameter
    fftTaskHandle = (TaskHandle_t) arg;
    // set play_music to 1
    play_music = 1;
    // wait for i2s_adc_capture_task to signal that adc can be disabled
    xTaskGenericNotifyWait(
      0 /* uxIndexToWaitOn */,
      0 /* ulBitsToClearOnEntry */,
      0 /* ulBitsToClearOnExit */,
      NULL /* pulNotificationValue */,
      portMAX_DELAY /* xTicksToWait*/
    );
    // disable adc
    // i2s_adc_disable(EXAMPLE_I2S_NUM);

    // allocate buffer for i2s read data
    uint8_t* i2s_write_buff = (uint8_t*) calloc(i2s_read_len, sizeof(char));

    //1. Play an example audio file(file format: 8bit/16khz/single channel)
    ESP_LOGI(TAG, "Playing file example: \n");
    int offset = 0;
    int tot_size = sizeof(audio_table);
    example_set_file_play_mode();
    while (offset < tot_size) {
        int play_len = ((tot_size - offset) > (4 * 1024)) ? (4 * 1024) : (tot_size - offset);
        int i2s_wr_len = example_i2s_dac_data_scale(i2s_write_buff, (uint8_t*)(audio_table + offset), play_len);
        i2s_write(EXAMPLE_I2S_NUM, i2s_write_buff, i2s_wr_len, &bytes_written, portMAX_DELAY);
        offset += play_len;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
    example_reset_play_mode();
    // free buffer
    free(i2s_write_buff);
    // set play_music to 0
    play_music = 0;
    // enable adc
    i2s_adc_enable(EXAMPLE_I2S_NUM);
    // restart i2s_adc_capture_task
    vTaskResume(adcTaskHandle);
    // restart fft task
    vTaskResume(fftTaskHandle);
    
    vTaskDelete(NULL);
}
