#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "espnow_mic.h"

static const char* TAG = "espnow_mic";

// number of frames to try and send at once (a frame is a left and right sample)
const int NUM_FRAMES_TO_SEND = 64;


uint8_t* mic_read_buf;
uint8_t* spk_write_buf;
uint8_t* audio_input_buf;  // buffer size same as esp now send packet size
uint8_t* audio_output_buf; // byte rate buffer size

static StreamBufferHandle_t network_stream_buf;
static StreamBufferHandle_t spk_stream_buf;


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
void i2s_std_playback_task(void* task_param) {
    // get the stream buffer handle from the task parameter
    StreamBufferHandle_t spk_read_buf = (StreamBufferHandle_t)task_param;

    i2s_chan_handle_t tx_chan = i2s_recv_std_config();

    size_t bytes_written = 0;

    // allocate memory for the read buffer
    spk_write_buf = (uint8_t*) calloc(EXAMPLE_I2S_READ_LEN, sizeof(char));
    assert(spk_write_buf != NULL);

    while (true) {
        // // read from the stream buffer, use errno to check if xstreambufferreceive is successful
        size_t num_bytes = xStreamBufferReceive(spk_read_buf, spk_write_buf, EXAMPLE_I2S_READ_LEN, portMAX_DELAY);

        assert (num_bytes == EXAMPLE_I2S_READ_LEN);
        esp_err_t err = i2s_channel_write(tx_chan, spk_write_buf, num_bytes, &bytes_written, portMAX_DELAY);
        if (err != ESP_OK) {
            printf("Error writing I2S: %0x\n", err);
            //exit the program
            exit(err);
        }else if (num_bytes == 0) {
            printf("Error reading from net stream buffer: %d\n", errno);
            ESP_LOGE(TAG, "No data in m");
        }
    }
    free(spk_write_buf);
    vTaskDelete(NULL);
}

/**
 * function to collect data from xStreamBufferRecieve until 128 frames of 250 bytes each are collected
 * process the data by scaling it to 8 bit, save it in audio_output_buf and return the pointer to the buffer
*/
void network_recv_task(void* task_param){

    // get the two stream buffer handles from the task parameter and cast it to the correct type
    StreamBufferHandle_t net_stream_buf = (StreamBufferHandle_t)task_param;

    // allocate memory for the read buffer
    audio_input_buf = (uint8_t*)calloc(READ_BUF_SIZE_BYTES, sizeof(char));
    assert(audio_input_buf != NULL);
    audio_output_buf = (uint8_t*) calloc(EXAMPLE_I2S_READ_LEN, sizeof(char)); // data for 2 channels
    assert(audio_input_buf != NULL);

    int packet_count = 0;
    int offset = 0;

    time_t start_time = time(NULL);

    while(true){
        
        // fill the audio_output_buf with data from the stream buffer until 128 frames of 250 bytes each are collected
        for ( int i = 0; i < NUM_FRAMES_TO_SEND; i++) {
            // read from the stream buffer, use errno to check if xstreambufferreceive is successful
            size_t num_bytes = xStreamBufferReceive(net_stream_buf, (void*) audio_input_buf, READ_BUF_SIZE_BYTES, portMAX_DELAY);
            if (num_bytes > 0) {
                // assert num_bytes is equal to the packet size, if false, exit the program
                assert(num_bytes == READ_BUF_SIZE_BYTES);
                // increment packet count
                packet_count++;
                // copy the data to the audio_output_buf at the correct offset
                memcpy(audio_output_buf + offset, audio_input_buf, num_bytes);
                // increment the offset
                offset += num_bytes;
            }
            else if (num_bytes == 0) {
                printf("Error reading from net stream buffer: %d\n", errno);
                ESP_LOGE(TAG, "No data in m");
            }
            else {
                printf("Other error reading from net stream: %d\n", errno);
                // exit with error code and error message
                exit(errno);
            }
        }

        // reset the offset
        offset = 0;

        // print the number of packets received every 10 seconds
        if(time(NULL) - start_time >= 10){
            printf("Received %d packets in %lld seconds\n", packet_count, time(NULL) - start_time);
            start_time = time(NULL);
            packet_count = 0;
        }

        // scale the data to 8 bit
        i2s_adc_data_scale(audio_output_buf, audio_output_buf, EXAMPLE_I2S_READ_LEN);
        
        // send audio_output_buf to dac with stream buffer
        size_t bytes_written = xStreamBufferSend(spk_stream_buf, (void*) audio_output_buf, EXAMPLE_I2S_READ_LEN, portMAX_DELAY);
        if (bytes_written != EXAMPLE_I2S_READ_LEN) {
            printf("Error writing to spk stream buffer: %d\n", errno);
            ESP_LOGE(TAG, "Error writing to spk stream buffer");
        }
    }
}

/* call the init_auidio function for starting adc and filling the buf -second */
esp_err_t init_audio_trans(StreamBufferHandle_t mic_stream_buf){ 
    printf("initializing i2s mic\n");

    /* thread for adc and filling the buf for the transmitter */
    xTaskCreate(i2s_adc_capture_task, "i2s_adc_capture_task", 4096, (void*) mic_stream_buf, 4, NULL); 

    return ESP_OK;
}

/* call the init_auidio function for starting adc and filling the buf -second */
esp_err_t init_audio_recv(StreamBufferHandle_t net_stream_buf, StreamBufferHandle_t spkout_stream_buf){ 
    printf("initializing i2s spk\n");
    network_stream_buf = net_stream_buf;
    spk_stream_buf = spkout_stream_buf;
    // creat a variable to hold two streambufferhandles and pass it to the network_recv_task with the xTaskCreate function
    xTaskCreate(network_recv_task, "network_recv_task", 4096, (void*) network_stream_buf, 4, NULL);
    // /* thread for filling the buf for the reciever and dac */
    xTaskCreate(i2s_std_playback_task, "i2s_pdm_playback_task", 4096, (void*) spk_stream_buf, 4, NULL);
    

    return ESP_OK;
}  
