#include <cassert>
#include <cstring>
#include <sys/lock.h>
#include <unistd.h>

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_st7796.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "nvs_flash.h"

#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"

namespace {

constexpr i2c_port_num_t kI2cPort = I2C_NUM_0;
constexpr gpio_num_t kI2cSdaPin = GPIO_NUM_8;
constexpr gpio_num_t kI2cSclPin = GPIO_NUM_7;

constexpr spi_host_device_t kLcdSpiHost = SPI2_HOST;
constexpr uint32_t kLcdPixelClockHz = 80 * 1000 * 1000;
constexpr gpio_num_t kLcdMosiPin = GPIO_NUM_1;
constexpr gpio_num_t kLcdSclkPin = GPIO_NUM_5;
constexpr gpio_num_t kLcdDcPin = GPIO_NUM_3;
constexpr gpio_num_t kLcdBacklightPin = GPIO_NUM_6;

constexpr int kLcdHorRes = 320;
constexpr int kLcdVerRes = 480;
constexpr size_t kLvglDrawBufferLines = 40;
constexpr uint32_t kLvglTickPeriodMs = 2;
constexpr uint32_t kLvglTaskStackSize = 6 * 1024;
constexpr UBaseType_t kLvglTaskPriority = 2;

constexpr ledc_timer_t kBacklightTimer = LEDC_TIMER_1;
constexpr ledc_mode_t kBacklightMode = LEDC_LOW_SPEED_MODE;
constexpr ledc_channel_t kBacklightChannel = LEDC_CHANNEL_0;
constexpr ledc_timer_bit_t kBacklightDutyRes = LEDC_TIMER_10_BIT;
constexpr uint32_t kBacklightDutyMax = 1024;
constexpr uint32_t kBacklightFrequencyHz = 5000;
constexpr uint8_t kBacklightPercent = 80;

static const char *TAG = "piupiu_main";
static _lock_t s_lvgl_lock;

i2c_master_bus_handle_t s_i2c_bus = nullptr;
i2c_master_dev_handle_t s_axp_dev = nullptr;
XPowersPMU s_power;
esp_lcd_panel_io_handle_t s_panel_io = nullptr;
esp_lcd_panel_handle_t s_panel = nullptr;
lv_disp_draw_buf_t s_draw_buf;
lv_disp_drv_t s_disp_drv;
lv_disp_t *s_display = nullptr;

void init_nvs()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void init_i2c()
{
    i2c_master_bus_config_t config = {};
    config.clk_source = I2C_CLK_SRC_DEFAULT;
    config.i2c_port = kI2cPort;
    config.scl_io_num = kI2cSclPin;
    config.sda_io_num = kI2cSdaPin;
    config.glitch_ignore_cnt = 7;
    config.flags.enable_internal_pullup = true;
    ESP_ERROR_CHECK(i2c_new_master_bus(&config, &s_i2c_bus));
}

int pmu_read(uint8_t, uint8_t reg_addr, uint8_t *data, uint8_t len)
{
    if (data == nullptr || len == 0) {
        return -1;
    }
    const esp_err_t ret = i2c_master_transmit_receive(s_axp_dev, &reg_addr, 1, data, len, -1);
    return ret == ESP_OK ? 0 : -1;
}

int pmu_write(uint8_t, uint8_t reg_addr, uint8_t *data, uint8_t len)
{
    if (data == nullptr || len == 0) {
        return -1;
    }
    uint8_t buffer[16] = {};
    if (len + 1 > sizeof(buffer)) {
        return -1;
    }
    buffer[0] = reg_addr;
    std::memcpy(buffer + 1, data, len);
    const esp_err_t ret = i2c_master_transmit(s_axp_dev, buffer, len + 1, -1);
    return ret == ESP_OK ? 0 : -1;
}

void init_power()
{
    i2c_device_config_t device_config = {};
    device_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    device_config.device_address = 0x34;
    device_config.scl_speed_hz = 400 * 1000;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(s_i2c_bus, &device_config, &s_axp_dev));

    if (!s_power.begin(AXP2101_SLAVE_ADDRESS, pmu_read, pmu_write)) {
        ESP_LOGE(TAG, "Init AXP2101 failed");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    s_power.setVbusVoltageLimit(XPOWERS_AXP2101_VBUS_VOL_LIM_4V36);
    s_power.setVbusCurrentLimit(XPOWERS_AXP2101_VBUS_CUR_LIM_1500MA);
    s_power.setSysPowerDownVoltage(2600);
    s_power.setDC1Voltage(3300);
    s_power.setDC2Voltage(1000);
    s_power.setDC3Voltage(3300);
    s_power.setDC4Voltage(1000);
    s_power.setDC5Voltage(3300);
    s_power.setALDO1Voltage(3300);
    s_power.setALDO2Voltage(3300);
    s_power.setALDO3Voltage(3300);
    s_power.setALDO4Voltage(3300);
    s_power.setBLDO1Voltage(1500);
    s_power.setBLDO2Voltage(2800);
    s_power.setCPUSLDOVoltage(1000);
    s_power.setDLDO1Voltage(3300);
    s_power.setDLDO2Voltage(3300);

    s_power.enableDC2();
    s_power.enableDC3();
    s_power.enableDC4();
    s_power.enableDC5();
    s_power.enableALDO1();
    s_power.enableALDO2();
    s_power.enableALDO3();
    s_power.enableALDO4();
    s_power.enableBLDO1();
    s_power.enableBLDO2();
    s_power.enableCPUSLDO();
    s_power.enableDLDO1();
    s_power.enableDLDO2();
    s_power.disableTSPinMeasure();
    s_power.enableVbusVoltageMeasure();
    s_power.enableBattVoltageMeasure();
    s_power.enableSystemVoltageMeasure();

    ESP_LOGI(TAG, "AXP2101 ready, chip id: 0x%x", s_power.getChipID());
}

void init_backlight()
{
    ledc_timer_config_t timer_config = {};
    timer_config.speed_mode = kBacklightMode;
    timer_config.timer_num = kBacklightTimer;
    timer_config.duty_resolution = kBacklightDutyRes;
    timer_config.freq_hz = kBacklightFrequencyHz;
    timer_config.clk_cfg = LEDC_AUTO_CLK;
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ledc_channel_config_t channel_config = {};
    channel_config.speed_mode = kBacklightMode;
    channel_config.channel = kBacklightChannel;
    channel_config.timer_sel = kBacklightTimer;
    channel_config.intr_type = LEDC_INTR_DISABLE;
    channel_config.gpio_num = kLcdBacklightPin;
    channel_config.duty = 0;
    channel_config.hpoint = 0;
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));

    const uint32_t duty = (kBacklightPercent * (kBacklightDutyMax - 1)) / 100;
    ESP_ERROR_CHECK(ledc_set_duty(kBacklightMode, kBacklightChannel, duty));
    ESP_ERROR_CHECK(ledc_update_duty(kBacklightMode, kBacklightChannel));
}

bool on_color_trans_done(esp_lcd_panel_io_handle_t, esp_lcd_panel_io_event_data_t *, void *user_ctx)
{
    lv_disp_flush_ready(static_cast<lv_disp_drv_t *>(user_ctx));
    return false;
}

void display_flush(lv_disp_drv_t *, const lv_area_t *area, lv_color_t *color_map)
{
    const int offset_x1 = area->x1;
    const int offset_y1 = area->y1;
    const int offset_x2 = area->x2;
    const int offset_y2 = area->y2;

    esp_lcd_panel_draw_bitmap(s_panel, offset_x1, offset_y1, offset_x2 + 1, offset_y2 + 1, color_map);
}

void init_lcd()
{
    spi_bus_config_t bus_config = {};
    bus_config.sclk_io_num = kLcdSclkPin;
    bus_config.mosi_io_num = kLcdMosiPin;
    bus_config.miso_io_num = GPIO_NUM_NC;
    bus_config.quadwp_io_num = GPIO_NUM_NC;
    bus_config.quadhd_io_num = GPIO_NUM_NC;
    bus_config.max_transfer_sz = kLcdHorRes * kLvglDrawBufferLines * sizeof(uint16_t);
    ESP_ERROR_CHECK(spi_bus_initialize(kLcdSpiHost, &bus_config, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_spi_config_t io_config = {};
    io_config.cs_gpio_num = GPIO_NUM_NC;
    io_config.dc_gpio_num = kLcdDcPin;
    io_config.spi_mode = 0;
    io_config.pclk_hz = kLcdPixelClockHz;
    io_config.trans_queue_depth = 10;
    io_config.lcd_cmd_bits = 8;
    io_config.lcd_param_bits = 8;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)kLcdSpiHost, &io_config, &s_panel_io));

    esp_lcd_panel_dev_config_t panel_config = {};
    panel_config.reset_gpio_num = GPIO_NUM_NC;
    panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR;
    panel_config.bits_per_pixel = 16;
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(s_panel_io, &panel_config, &s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(s_panel, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(s_panel, true, false));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(s_panel, true));
}

void lvgl_tick(void *)
{
    lv_tick_inc(kLvglTickPeriodMs);
}

void lvgl_task(void *)
{
    const uint32_t threshold_ms = 1000 / CONFIG_FREERTOS_HZ;
    while (true) {
        _lock_acquire(&s_lvgl_lock);
        uint32_t delay_ms = lv_timer_handler();
        _lock_release(&s_lvgl_lock);
        delay_ms = delay_ms < threshold_ms ? threshold_ms : delay_ms;
        usleep(delay_ms * 1000);
    }
}

void init_lvgl()
{
    lv_init();

    const size_t buffer_size = kLcdHorRes * kLvglDrawBufferLines * sizeof(lv_color_t);
    void *buf1 = heap_caps_malloc(buffer_size, MALLOC_CAP_DMA | MALLOC_CAP_SPIRAM);
    void *buf2 = heap_caps_malloc(buffer_size, MALLOC_CAP_DMA | MALLOC_CAP_SPIRAM);
    assert(buf1 != nullptr);
    assert(buf2 != nullptr);

    lv_disp_draw_buf_init(&s_draw_buf, buf1, buf2, kLcdHorRes * kLvglDrawBufferLines);
    lv_disp_drv_init(&s_disp_drv);
    s_disp_drv.hor_res = kLcdHorRes;
    s_disp_drv.ver_res = kLcdVerRes;
    s_disp_drv.flush_cb = display_flush;
    s_disp_drv.draw_buf = &s_draw_buf;
    s_disp_drv.user_data = s_panel;
    s_display = lv_disp_drv_register(&s_disp_drv);

    const esp_lcd_panel_io_callbacks_t callbacks = {
        .on_color_trans_done = on_color_trans_done,
    };
    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(s_panel_io, &callbacks, &s_disp_drv));

    const esp_timer_create_args_t timer_args = {
        .callback = &lvgl_tick,
        .name = "lvgl_tick",
    };
    esp_timer_handle_t lvgl_tick_timer = nullptr;
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, kLvglTickPeriodMs * 1000));

    xTaskCreate(lvgl_task, "lvgl", kLvglTaskStackSize, nullptr, kLvglTaskPriority, nullptr);
}

void create_ui()
{
    _lock_acquire(&s_lvgl_lock);

    lv_obj_t *screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x101820), 0);
    lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x1F5A7A), 0);
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "piupiu prototype");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 48);

    lv_obj_t *status = lv_label_create(screen);
    lv_label_set_text(status, "AXP2101 / ST7796 / LVGL ready");
    lv_obj_set_style_text_color(status, lv_palette_lighten(LV_PALETTE_BLUE, 3), 0);
    lv_obj_set_style_text_font(status, &lv_font_montserrat_14, 0);
    lv_obj_align(status, LV_ALIGN_TOP_MID, 0, 88);

    lv_obj_t *panel = lv_obj_create(screen);
    lv_obj_set_size(panel, 260, 180);
    lv_obj_align(panel, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_style_radius(panel, 18, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xF2F7FB), 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_shadow_width(panel, 24, 0);
    lv_obj_set_style_shadow_opa(panel, LV_OPA_20, 0);
    lv_obj_set_style_pad_all(panel, 20, 0);

    lv_obj_t *hint = lv_label_create(panel);
    lv_label_set_text(hint, "Board pins and PMU setup are aligned with LCDdemo 02_lvgl_example.");
    lv_label_set_long_mode(hint, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(hint, 220);
    lv_obj_set_style_text_color(hint, lv_color_hex(0x23313F), 0);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_14, 0);
    lv_obj_align(hint, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *footer = lv_label_create(panel);
    lv_label_set_text(footer, "Touch is not wired in this offline build yet.");
    lv_label_set_long_mode(footer, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(footer, 220);
    lv_obj_set_style_text_color(footer, lv_palette_darken(LV_PALETTE_GREY, 1), 0);
    lv_obj_set_style_text_font(footer, &lv_font_montserrat_14, 0);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    _lock_release(&s_lvgl_lock);
}

} // namespace

extern "C" void app_main()
{
    ESP_LOGI(TAG, "Piupiu firmware starting");

    init_nvs();
    init_i2c();
    init_power();
    init_lcd();
    init_backlight();
    init_lvgl();
    create_ui();
}
