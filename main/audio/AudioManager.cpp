#include "AudioManager.h"
#include "driver/i2s_std.h"
#include "driver/i2s_types.h"
#include "esp_afe_config.h"

AudioManager::AudioManager() : _afe_handle(nullptr), _afe_data(nullptr), _i2s_buff(nullptr), _i2s_buff_len(0), _onWakeup(nullptr), _audioTaskHandle(nullptr) {
}

AudioManager::~AudioManager() {
    if (_i2s_buff) {
        free(_i2s_buff);
    }
    if (_afe_handle && _afe_data) {
        _afe_handle->destroy(_afe_data);
    }
}

bool AudioManager::begin() {

    _i2s.setPins(I2S_MIC_SCK, I2S_MIC_WS, -1, I2S_MIC_SD);
    
    if (!_i2s.begin(I2S_MODE_STD, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT)) {
        return false;
    }

    const esp_partition_t *part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "model");
    if (part == NULL) {
        Serial.println("AudioManager: Warning! 'model' partition not found. WakeNet may fail.");
    } else {
        Serial.println("AudioManager: 'model' partition found.");
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
        Serial.println("AudioManager: Failed to create AFE handle!");
        return false;
    }
    Serial.println("AudioManager: AFE initialized");

    int chunk_size = _afe_handle->get_feed_chunksize(_afe_data);
    _i2s_buff_len = chunk_size * sizeof(int16_t); 
    _i2s_buff = (int16_t *)malloc(_i2s_buff_len);
    
    if (_i2s_buff == NULL) {
        Serial.println("AudioManager: Failed to allocate I2S buffer!");
        return false;
    }
    Serial.printf("AudioManager: I2S buffer allocated size: %d bytes\n", _i2s_buff_len);

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
        Serial.println("AudioManager: Failed to create task!");
        return false;
    }

    Serial.println("AudioManager: Setup complete");
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
        
        bytes_read = am->_i2s.readBytes((char *)am->_i2s_buff, am->_i2s_buff_len);
        
        if (bytes_read == am->_i2s_buff_len) {
            int ret_size = am->_afe_handle->feed(am->_afe_data, am->_i2s_buff);
            
            if (ret_size > 0) {
                 afe_fetch_result_t* res = am->_afe_handle->fetch(am->_afe_data);
                 if (res && res->wakeup_state == WAKENET_DETECTED) {
                    Serial.println(">>> AudioManager: WAKEWORD DETECTED! <<<");
                    if (am->_onWakeup) {
                        am->_onWakeup();    
                    }
                 }
            }
        } else {
            vTaskDelay(1);
        }
    }
}
