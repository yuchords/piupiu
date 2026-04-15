# piupiu

## 项目介绍

`piupiu` 是一个基于 ESP32 的桌面 AI 助手项目。

当前阶段聚焦于 `ESP32 + ESP-IDF` 的实验原型验证，主要目标是先完成核心硬件能力和基础软件链路的可行性确认。原型硬件方向包括 USB Hub、LED 灯带、扬声器、麦克风、摄像头等桌面助手相关外设。

在整体规划上，项目后续会扩展到服务端、微信小程序，以及更远期的桌面端和移动端。但当前仓库的主开发重点仍然是：
- ESP32 原型固件
- 服务端预留
- 文档与架构沉淀

## 主目录结构

### `.agent/`

存放面向 AI Agent 的补充约束和参考文档，仅在相关任务下按需读取。

关键文件：
- `DESIGN.md`：设计、架构、选型、模块规划相关参考
- `changelog.md`：大版本更新日志的撰写规范

### `docs/`

项目正式文档目录，用于沉淀接口、架构和版本信息。

关键子目录：
- `docs/api/`：接口说明，覆盖嵌入式端、服务端以及前后端通信相关约定
- `docs/architecture/`：架构设计、硬件选型、软件功能规划
- `docs/changelog/`：阶段性与大版本更新日志

关键文件：
- `docs/README.md`：`docs` 目录总览说明

### `esp_prototype/`

当前最核心的开发目录，存放 ESP32 实验原型代码，基于 ESP-IDF。

关键子目录：
- `esp_prototype/components/`：项目自定义组件
- `esp_prototype/lib/`：第三方或外部库
- `esp_prototype/main/`：主程序入口和核心业务逻辑
- `esp_prototype/managed_components/`：由 ESP-IDF 管理的组件依赖

关键文件：
- `esp_prototype/README.md`：ESP32 原型说明
- `esp_prototype/CMakeLists.txt`：ESP-IDF/CMake 构建入口
- `esp_prototype/dependencies.lock`：组件依赖锁定文件
- `esp_prototype/partitions.csv`：分区表配置
- `esp_prototype/sdkconfig`：当前构建配置
- `esp_prototype/sdkconfig.defaults`：默认配置模板
- `esp_prototype/espidf.bat`：本地 ESP-IDF 相关脚本入口

### `script/`

用于放置仓库级辅助脚本，例如构建、清理、检查、发布、数据处理等脚本。

关键文件：
- `script/README.md`：脚本目录说明

### `server/`

服务端预留目录。

当前阶段主要用于后续服务端开发准备，后续会承载与 ESP32 原型和微信小程序配套的接口、业务逻辑和部署内容。




### `AGENTS.md`

仓库级 Agent 工作约束文件，定义项目范围、编码风格、文档路由和按需读取规则。



## 当前阶段说明

目前仓库以 ESP32 实验原型验证为中心，服务端和微信小程序属于近阶段规划，桌面端、Android、iOS 暂不作为当前实现重点。
