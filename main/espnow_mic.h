#ifndef ESPNOW_MIC_H
#define ESPNOW_MIC_H

#include "config.h"

void i2s_adc_capture_task(void* task_param);
void i2s_dac_playback_task(void* task_param);
void i2s_pdm_playback_task(void* task_param);
esp_err_t init_audio_trans(StreamBufferHandle_t mic_stream_buf);
esp_err_t init_audio_recv(StreamBufferHandle_t network_stream_buf);
void example_erase_flash(void);

#endif