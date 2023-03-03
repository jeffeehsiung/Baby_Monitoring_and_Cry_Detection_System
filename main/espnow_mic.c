#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"
#include "esp_vfs_fat.h"
#include "esp_adc_cal.h"
#include "esp_partition.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/stream_buffer.h"
#include "driver/i2s.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"
#include "spi_flash_mmap.h"
#include "espnow_mic.h"
#include "errno.h"

static const char* TAG = "espnow_mic";

// analog microphone Settings - ADC1_CHANNEL_7 is GPIO35
#define ADC1_TEST_CHANNEL (ADC1_CHANNEL_7)
// i2s mic and adc settings
#define V_REF   1100
//enable record sound and save in flash
#define RECORD_IN_FLASH_EN        (1)
//enable replay recorded sound in flash
#define REPLAY_FROM_FLASH_EN      (1)

//i2s number for interface channel (0 or 1, 0 for mic and 1 for speaker)
#define EXAMPLE_I2S_NUM           (0)
//i2s sample rate
#define EXAMPLE_I2S_SAMPLE_RATE   (16000)
//i2s data bits
#define EXAMPLE_I2S_SAMPLE_BITS   (16)
//I2S data format
#define EXAMPLE_I2S_FORMAT        (I2S_CHANNEL_FMT_ONLY_RIGHT)
//I2S channel number
#define EXAMPLE_I2S_CHANNEL_NUM   ((EXAMPLE_I2S_FORMAT < I2S_CHANNEL_FMT_ONLY_RIGHT) ? (2) : (1))
//I2S built-in ADC unit
#define I2S_ADC_UNIT              ADC_UNIT_1
//I2S built-in ADC channel GPIO36
#define I2S_ADC_CHANNEL           ADC1_CHANNEL_0
// SPI DMA channel
#define SPI_DMA_CHAN SPI_DMA_CH_AUTO
// define max read buffer size
#define READ_BUF_SIZE_BYTES       (250)

static uint8_t mic_read_buf[READ_BUF_SIZE_BYTES];
static uint8_t audio_output_buf[READ_BUF_SIZE_BYTES];

/** ----------------------SD CARD---------------------------------*/
#define SPI_MOSI_GPIO 23
#define SPI_MISO_GPIO 19
#define SPI_CLK_GPIO 18
#define SPI_CS_GPIO 5
#define SD_MOUNT_POINT "/sdcard"
#define NUM_CHANNELS 1
#define BYTE_RATE (EXAMPLE_I2S_SAMPLE_RATE * (8 / 8)) * NUM_CHANNELS

// When testing SD and SPI modes, keep in mind that once the card has been
// initialized in SPI mode, it can not be reinitialized in SD mode without
// toggling power to the card.
sdmmc_host_t host = SDSPI_HOST_DEFAULT();
sdmmc_card_t *card;
const int WAVE_HEADER_SIZE = 44;

/**
 * @brief Initializes the slot without card detect (CD) and write protect (WP) signals.
 * It formats the card if mount fails and initializes the card. After the card has been
 * initialized, it print the card properties
 */
void mount_sdcard(void)
{
    esp_err_t ret;
    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 8 * 128};
    ESP_LOGI(TAG, "Initializing SD card");

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SPI_MOSI_GPIO,
        .miso_io_num = SPI_MISO_GPIO,
        .sclk_io_num = SPI_CLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SPI_CS_GPIO;
    slot_config.host_id = host.slot;

    ret = esp_vfs_fat_sdspi_mount(SD_MOUNT_POINT, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount filesystem.");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                          "Make sure SD card lines have pull-up resistors in place.",
                     esp_err_to_name(ret));
        }
        return;
    }
    sdmmc_card_print_info(stdout, card); // Card has been initialized, print its properties
}

/**
 * @brief Generates the header for the WAV file that is going to be stored in the SD card.
 * See this for reference: http://soundfile.sapp.org/doc/WaveFormat/.
 */
void generate_wav_header(char *wav_header, uint32_t wav_size, uint32_t sample_rate)
{
    uint32_t file_size = wav_size + WAVE_HEADER_SIZE - 8;
    uint32_t byte_rate = BYTE_RATE;

    const char set_wav_header[] = {
        'R', 'I', 'F', 'F',                                                  // ChunkID
        file_size, file_size >> 8, file_size >> 16, file_size >> 24,         // ChunkSize
        'W', 'A', 'V', 'E',                                                  // Format
        'f', 'm', 't', ' ',                                                  // Subchunk1ID
        0x10, 0x00, 0x00, 0x00,                                              // Subchunk1Size (16 for PCM)
        0x01, 0x00,                                                          // AudioFormat (1 for PCM)
        0x01, 0x00,                                                          // NumChannels (1 channel)
        sample_rate, sample_rate >> 8, sample_rate >> 16, sample_rate >> 24, // SampleRate
        byte_rate, byte_rate >> 8, byte_rate >> 16, byte_rate >> 24,         // ByteRate
        0x02, 0x00,                                                          // BlockAlign
        0x10, 0x00,                                                          // BitsPerSample (16 bits)
        'd', 'a', 't', 'a',                                                  // Subchunk2ID
        wav_size, wav_size >> 8, wav_size >> 16, wav_size >> 24,             // Subchunk2Size
    };

    memcpy(wav_header, set_wav_header, sizeof(set_wav_header));
}
/** ------------------------------END OF SD CARD-------------------------------------------*/

/**
 * @brief I2S config for using internal ADC and DAC
 * one time set up
 */
void i2s_common_config(void)
{
     int i2s_num = EXAMPLE_I2S_NUM;
     i2s_config_t i2s_config = {
        // .mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN | I2S_MODE_ADC_BUILT_IN, // master and rx for mic, tx for speaker, adc for internal adc
        .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN, // master and rx for mic, tx for speaker, adc for internal adc
        .sample_rate =  EXAMPLE_I2S_SAMPLE_RATE, // 16KHz for adc
        .bits_per_sample = EXAMPLE_I2S_SAMPLE_BITS, // 16 bits for adc
        .communication_format = I2S_COMM_FORMAT_STAND_MSB, // standard format for adc
        .channel_format = EXAMPLE_I2S_FORMAT, // only right channel for adc
        .intr_alloc_flags = 0, // default interrupt priority
        .dma_desc_num = 6, // number of dma descriptors, or count for adc
        .dma_frame_num = 256, // number of dma frames, or length for adc
        .use_apll = 1, // meaning using ethernet data interface framework
        .tx_desc_auto_clear = false, // i2s auto clear tx descriptor on underflow
        .fixed_mclk = 0, // i2s fixed MLCK clock
     };
     //install and start i2s driver
     i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
     //init ADC pad
     i2s_set_adc_mode(I2S_ADC_UNIT, I2S_ADC_CHANNEL);
     //init DAC pad
     i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN); // enable both I2S built-in DAC channels L/R
}

/**
 * @brief Scale data to 8bit for data from ADC.
 *        Data from ADC are 12bit width by default.
 *        DAC can only output 8 bit data.
 *        Scale each 12bit ADC data to 8bit DAC data.
 */
void i2s_adc_data_scale(uint8_t * des_buff, uint8_t* src_buff, uint8_t len) // debug log: change uint32_t to uint8_t
{
    uint8_t j = 0;
    uint8_t dac_value = 0;
#if (EXAMPLE_I2S_SAMPLE_BITS == 16)
    for (int i = 0; i < len; i += 2) {
        dac_value = ((((uint16_t) (src_buff[i + 1] & 0xf) << 8) | ((src_buff[i + 0]))));
        des_buff[j++] = 0;
        des_buff[j++] = dac_value * 256 / 4096;
    }
#endif
}

// i2s dac playback task
void i2s_dac_playback_task(void* task_param) {
    StreamBufferHandle_t net_stream_buf = (StreamBufferHandle_t)task_param;
    size_t bytes_written = 0;
    TickType_t ticks_to_wait = 100; // wait 100 ticks for the net_stream_buf to be available
    while (true) {
        size_t num_bytes = xStreamBufferReceive(net_stream_buf, (void*)audio_output_buf,sizeof(audio_output_buf), portMAX_DELAY);
        if (num_bytes > 0) {
            ESP_LOGI(TAG, "Read %d bytes from net_stream_buf", num_bytes);
            // process data and scale to 8bit for I2S DAC.
            // i2s_adc_data_scale(audio_output_buf, audio_output_buf, num_bytes);
            // send data to i2s dac
            esp_err_t err = i2s_write(EXAMPLE_I2S_NUM, audio_output_buf, num_bytes, &bytes_written, portMAX_DELAY);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Error writing to i2s dac: %d", errno);
            }
            else {
                ESP_LOGI(TAG, "Sent %d bytes to i2s dac", num_bytes);
            }
        }
        else {
            ESP_LOGE(TAG, "Error reading %d bytes from net_stream_buf", num_bytes);
        }
    }
}

/* initialize configuration of mic -first */
esp_err_t i2s_audio_init (void){
    i2s_common_config();
    esp_log_level_set("I2S", ESP_LOG_INFO);
    return ESP_OK;
}

// i2s dac playback task
void i2s_dac_playback_task_new(void* task_param) {
    /** -------------------------------------------------------*/
    mount_sdcard();
    // Use POSIX and C standard library functions to work with files.
    int flash_wr_size = 0;
    int rec_time = 5; // seconds
    char wav_header_fmt[WAVE_HEADER_SIZE];
    uint32_t flash_rec_size = BYTE_RATE * rec_time;
    generate_wav_header(wav_header_fmt, flash_rec_size, EXAMPLE_I2S_SAMPLE_RATE);

    // First check if file exists before creating a new file.
    struct stat st;
    if (stat(SD_MOUNT_POINT "/record.wav", &st) == 0)
    {
        // Delete it if it exists
        unlink(SD_MOUNT_POINT "/record.wav");
    }
    // Create new WAV file
    FILE *f = fopen(SD_MOUNT_POINT "/record.wav", "a");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        vTaskDelete(NULL);
    }
    // Write the header to the WAV file
    fwrite(wav_header_fmt, 1, WAVE_HEADER_SIZE, f);
    /** -------------------------------------------------------*/

    StreamBufferHandle_t net_stream_buf = (StreamBufferHandle_t)task_param;
    size_t bytes_written = 0;
    TickType_t ticks_to_wait = 100; // wait 100 ticks for the net_stream_buf to be available
    while (flash_wr_size < flash_rec_size) {
        size_t num_bytes = xStreamBufferReceive(net_stream_buf, (void*)audio_output_buf,sizeof(audio_output_buf), ticks_to_wait);
        if (num_bytes > 0) {
            ESP_LOGI(TAG, "Read %d bytes from net_stream_buf", num_bytes);
            // process data and scale to 8bit for I2S DAC.
            // i2s_adc_data_scale(audio_output_buf, audio_output_buf, num_bytes);
            // send data to i2s dac
            // esp_err_t err = i2s_write(EXAMPLE_I2S_NUM, audio_output_buf, num_bytes, &bytes_written, portMAX_DELAY);
            // if (err != ESP_OK) {
            //     ESP_LOGE(TAG, "Error writing to i2s dac: %d", errno);
            // }
            // else {
            //     ESP_LOGI(TAG, "Sent %d bytes to i2s dac", num_bytes);
            // }
            fwrite(audio_output_buf, 1, num_bytes, f);
            flash_wr_size += num_bytes;
            ESP_LOGI(TAG, "Wrote %d/%ld bytes to file - %ld%%", flash_wr_size, flash_rec_size, (flash_wr_size * 100) / flash_rec_size);
        }
        else {
            ESP_LOGE(TAG, "Error reading %d bytes from net_stream_buf", num_bytes);
        }
    }
    ESP_LOGI(TAG, "Recording done!");
    fclose(f);
    ESP_LOGI(TAG, "File written on SDCard");

    // All done, unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, card);
    ESP_LOGI(TAG, "Card unmounted");
    // Deinitialize the bus after all devices are removed
    spi_bus_free(host.slot);
    // delete task after recording
    vTaskDelete(NULL);
}

/* call the init_auidio function for starting adc and filling the buf -second */
esp_err_t init_audio(StreamBufferHandle_t mic_stream_buf, StreamBufferHandle_t network_stream_buf){ 
    printf("initializing i2s mic\n");
    i2s_audio_init();

    /* thread for adc and filling the buf for the transmitter */
    // xTaskCreate(i2s_adc_capture_task, "i2s_adc_capture_task", 2048, (void*) mic_stream_buf, 4, NULL); 
    /* thread for filling the buf for the reciever and dac */
    xTaskCreate(i2s_dac_playback_task_new, "i2s_dac_playback_task", 4096, (void*) network_stream_buf, 4, NULL);
    /* adc analog voltage calibration */
    // xTaskCreate(adc_cali_read_task, "adc_cali_read_task", 2048, NULL, 4, NULL);

    return ESP_OK;
}
