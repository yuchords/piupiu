# 音频服务模块 (Audio Service)

## 1. 模块简介
本模块 (`audio`) 负责处理 ESP32-S3 的音频输入与语音识别功能。核心功能是实现**离线语音唤醒**，使用 Espressif 官方的 **ESP-SR** 组件进行唤醒词检测（WakeNet），并利用 **Arduino 框架** 简化底层的 I2S 驱动开发。

该模块封装了一个简单易用的 `AudioManager` 类，屏蔽了底层复杂的音频流处理和算法配置，向上层应用提供简单的事件回调接口。

## 2. 核心组件
*   **AudioManager**: 核心管理类，负责 I2S 初始化、ESP-SR 算法引擎配置及后台音频处理任务。
*   **ESP-SR**: 乐鑫提供的语音识别库，包含 AFE（音频前端处理：降噪、回声消除等）和 WakeNet（唤醒词检测）。
*   **ESP_I2S**: Arduino 风格的 I2S 驱动库，用于从麦克风读取原始音频数据。

## 3. 硬件依赖
模块默认配置如下（可在 `AudioManager.h` 中修改）：
*   **麦克风接口**: I2S 标准模式 (Standard Mode)
*   **采样率**: 16kHz
*   **位宽**: 16-bit
*   **通道**: 单声道 (Mono)
*   **引脚定义**:
    *   `I2S_MIC_SCK` (BCLK): GPIO 42
    *   `I2S_MIC_WS` (LRCK): GPIO 41
    *   `I2S_MIC_SD` (DOUT): GPIO 2

> **注意**: 请务必根据实际电路板修改 `AudioManager.h` 中的引脚宏定义。

## 4. 软件依赖
需要在 `idf_component.yml` 中添加以下依赖：
```yaml
dependencies:
  espressif/esp-sr: "^1.0.0" # 核心语音识别库
```
同时依赖 Arduino 核心库 (`arduino-esp32`)。

## 5. 使用指南

### 5.1 初始化
在系统启动代码（如 `setup()` 或应用初始化函数）中实例化并启动 `AudioManager`：

```cpp
#include "audio/AudioManager.h"

AudioManager audio;

// 定义唤醒回调函数
void onWakeupCallback() {
    Serial.println("收到唤醒词！AI 开始聆听...");
    // TODO: 在此处触发系统状态机，例如切换到 AI_LISTENING 状态
}

void setup() {
    Serial.begin(115200);
    
    // 设置唤醒回调
    audio.setWakeupCallback(onWakeupCallback);

    // 启动音频服务
    if (!audio.begin()) {
        Serial.println("音频服务启动失败！请检查 I2S 引脚或 ESP-SR 模型分区。");
    } else {
        Serial.println("音频服务已就绪，请说出唤醒词...");
    }
}

void loop() {
    // 主循环无需处理音频，所有工作在后台 FreeRTOS 任务中自动完成
    delay(1000);
}
```

### 5.2 关键 API 说明
*   **`bool begin()`**:
    *   初始化 I2S 总线。
    *   加载 ESP-SR 模型（需确保 Flash 中有 `model` 分区）。
    *   创建并启动后台音频处理任务 (`AudioTask`)。
    *   返回 `true` 表示成功。
*   **`void setWakeupCallback(WakeupCallback cb)`**:
    *   注册一个无参数、无返回值的回调函数。
    *   当检测到唤醒词（如 "Hi ESP"）时，该函数会被自动调用。

## 6. 分区表要求
ESP-SR 算法依赖存储在 Flash 中的模型数据。请确保项目的分区表 (`partitions.csv`) 中包含名为 `model` 的数据分区，例如：

```csv
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     ,        0x4000,
otadata,  data, ota,     ,        0x2000,
phy_init, data, phy,     ,        0x1000,
model,    data, spiffs,  ,        4M,      # 必须包含此分区用于存放语音模型
factory,  app,  factory, ,        2M,
```

## 7. 调试与排错
*   **初始化失败**: 检查串口日志。
    *   `Failed to initialize I2S!`: 检查引脚是否冲突。
    *   `'model' partition not found`: 检查分区表是否正确烧录。
*   **无法唤醒**:
    *   确认麦克风硬件是否正常（可尝试录制一段原始音频进行验证）。
    *   环境噪音是否过大。
    *   尝试调高说话音量或靠近麦克风。
