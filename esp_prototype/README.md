# ESP32 测试原型程序

这是 `esp32 prototype` 的 ESP-IDF 工程。

当前已经接入 `LCDdemo` 里的 `ESP32-S3-Touch-LCD-3.5-Demo/ESP-IDF/02_lvgl_example` 板级配置，完成了当前这条可离线构建的最小原型启动链路：

- AXP2101 供电初始化
- 3.5 寸 LCD 初始化
- LVGL 显示初始化
- 原型机启动测试界面

## 说明

板级本地组件通过工程级 `EXTRA_COMPONENT_DIRS` 直接指向 demo 内已经存在的本地驱动。

当前没有把 FT6336 触摸一起接入构建，因为该部分依赖额外的 Espressif Registry 组件，而当前环境无法联网拉取。显示和电源链路已经可以先作为 prototype 的基础配置。

## 构建

在 `esp_prototype` 目录下执行：

```powershell
cmd /c "espidf.bat && set IDF_COMPONENT_MANAGER=0 && idf.py build"
cmd /c "espidf.bat && set IDF_COMPONENT_MANAGER=0 && idf.py flash monitor"
```

