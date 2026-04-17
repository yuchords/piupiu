#include "AudioManager.h"
#include "esp_afe_config.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_partition.h"
#include <cstdlib>

namespace {

static const char *TAG = "AudioManager";

}

AudioManager::AudioManager()
    : _i2sRxHandle(nullptr),
      _afe_handle(nullptr),
      _afe_data(nullptr),
      _i2s_buff(nullptr),
      _i2s_buff_len(0),
      _onWakeup(nullptr),
      _audioTaskHandle(nullptr) {
}

AudioManager::~AudioManager() {
    if (_audioTaskHandle) {
        vTaskDelete(_audioTaskHandle);
        _audioTaskHandle = nullptr;
    }

    if (_i2s_buff) {
        free(_i2s_buff);
        _i2s_buff = nullptr;
    }

    if (_afe_handle && _afe_data) {
        _afe_handle->destroy(_afe_data);
        _afe_data = nullptr;
    }

    if (_i2sRxHandle) {
        i2s_channel_disable(_i2sRxHandle);
        i2s_del_channel(_i2sRxHandle);
        _i2sRxHandle = nullptr;
    }
}

bool AudioManager::begin() {
    if (_i2sRxHandle != nullptr) {
        ESP_LOGW(TAG, "I2S already initialized");
        return true;
    }

    const i2s_chan_config_t chanCfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    esp_err_t err = i2s_new_channel(&chanCfg, nullptr, &_i2sRxHandle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2S RX channel: %s", esp_err_to_name(err));
        return false;
    }

    const i2s_std_config_t stdCfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = static_cast<gpio_num_t>(I2S_MIC_SCK),
            .ws = static_cast<gpio_num_t>(I2S_MIC_WS),
            .dout = I2S_GPIO_UNUSED,
            .din = static_cast<gpio_num_t>(I2S_MIC_SD),
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    err = i2s_channel_init_std_mode(_i2sRxHandle, &stdCfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init I2S std mode: %s", esp_err_to_name(err));
        return false;
    }

    err = i2s_channel_enable(_i2sRxHandle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable I2S RX channel: %s", esp_err_to_name(err));
        return false;
    }

    const esp_partition_t *part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "model");
    if (part == NULL) {
        ESP_LOGW(TAG, "'model' partition not found. WakeNet may fail.");
    } else {
        ESP_LOGI(TAG, "'model' partition found.");
    }

    afe_config_t afe_config = AFE_CONFIG_DEFAULT();
    
    afe_config.wakenet_init = true;
    afe_config.wakenet_model_name = NULL; // 使用默认模型 (通常是 Hi ESP)
    afe_config.wakenet_mode = DET_MODE_90; 

    afe_config.pcm_config.total_ch_num = 1;
    afe_config.pcm_config.mic_num = 1;
    afe_config.pcm_config.ref_num = 0;
    
    afe_config.memory_alloc_mode = AFE_MEMORY_ALLOC_MORE_PSRAM;
    afe_config.voice_communication_init = false; 

    _afe_handle = &ESP_AFE_SR_HANDLE;
    
    _afe_data = _afe_handle->create_from_config(&afe_config);

    if (_afe_data == NULL) {
        ESP_LOGE(TAG, "Failed to create AFE handle");
        return false;
    }
    ESP_LOGI(TAG, "AFE initialized");

    int chunk_size = _afe_handle->get_feed_chunksize(_afe_data);
    _i2s_buff_len = chunk_size * sizeof(int16_t); 
    _i2s_buff = (int16_t *)malloc(_i2s_buff_len);
    
    if (_i2s_buff == NULL) {
        ESP_LOGE(TAG, "Failed to allocate I2S buffer");
        return false;
    }
    ESP_LOGI(TAG, "I2S buffer allocated: %d bytes", _i2s_buff_len);

    BaseType_t ret = xTaskCreatePinnedToCore(
        AudioManager::audioTask,
        "AudioTask",
        4 * 1024,
        this,
        5,
        &_audioTaskHandle,
        1
    );

    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create audio task");
        return false;
    }

    ESP_LOGI(TAG, "Setup complete");
    return true;
}

void AudioManager::update() {
    
}

void AudioManager::setWakeupCallback(WakeupCallback cb) {
    _onWakeup = cb;
}

void AudioManager::audioTask(void *arg) {
    AudioManager *am = (AudioManager *)arg;
    
    while (true) {
        size_t bytes_read = 0;

        esp_err_t err = i2s_channel_read(
            am->_i2sRxHandle,
            am->_i2s_buff,
            am->_i2s_buff_len,
            &bytes_read,
            pdMS_TO_TICKS(100)
        );

        if (err == ESP_OK && bytes_read == static_cast<size_t>(am->_i2s_buff_len)) {
            int ret_size = am->_afe_handle->feed(am->_afe_data, am->_i2s_buff);
            
            if (ret_size > 0) {
                 afe_fetch_result_t* res = am->_afe_handle->fetch(am->_afe_data);
                 if (res && res->wakeup_state == WAKENET_DETECTED) {
                    ESP_LOGI(TAG, "Wake word detected");
                    if (am->_onWakeup) {
                        am->_onWakeup();    
                    }
                 }
            }
        } else {
            if (err != ESP_OK && err != ESP_ERR_TIMEOUT) {
                ESP_LOGW(TAG, "I2S read failed: %s", esp_err_to_name(err));
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}
