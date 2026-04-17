#pragma once

#include <cstdint>
#include "driver/i2s_std.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_afe_sr_iface.h"
#include "esp_afe_sr_models.h"
#include "esp_wn_iface.h"
#include "esp_wn_models.h"

// 麦克风引脚定义
#define I2S_MIC_SCK 42
#define I2S_MIC_WS  41
#define I2S_MIC_SD  2

// 采样配置
#define SAMPLE_RATE 16000
#define CHANNELS    1 // 单麦克风

typedef void (*WakeupCallback)(void);

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    bool begin();

    void update();

    void setWakeupCallback(WakeupCallback cb);

private:
    i2s_chan_handle_t _i2sRxHandle;
    
    const esp_afe_sr_iface_t *_afe_handle;
    esp_afe_sr_data_t *_afe_data;

    int16_t *_i2s_buff;
    int _i2s_buff_len;

    WakeupCallback _onWakeup;

    TaskHandle_t _audioTaskHandle;
    
    static void audioTask(void *arg);
};
