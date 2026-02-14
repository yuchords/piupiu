#include "AppLog.h"
#include "AppManagerLog.h"

AppLog::AppLog() : _lastLogCount(0), _simTimer(nullptr) {
    _name = "Log";
}

AppLog::~AppLog() {
}

void AppLog::onViewLoad() {
    // 1. 创建黑色背景根容器
    _root = lv_obj_create(lv_scr_act());
    lv_obj_set_size(_root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(_root, lv_color_black(), 0);
    lv_obj_set_scrollbar_mode(_root, LV_SCROLLBAR_MODE_OFF);

    // 2. 标题栏
    lv_obj_t* title = lv_label_create(_root);
    lv_label_set_text(title, "System Logs");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0); // 确保你的lv_conf.h开启了这个字体
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // 3. 日志列表容器
    _cont = lv_obj_create(_root);
    lv_obj_set_size(_cont, LV_PCT(100), LV_PCT(85));
    lv_obj_align(_cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(_cont, lv_color_black(), 0);
    lv_obj_set_style_border_width(_cont, 0, 0);
    lv_obj_set_flex_flow(_cont, LV_FLEX_FLOW_COLUMN); // 垂直布局
    lv_obj_set_style_pad_all(_cont, 10, 0);
    lv_obj_set_style_pad_row(_cont, 8, 0); // 行间距

    // 注册回调，当有新日志时刷新界面
    AppManagerLog::setUpdateCallback([this](){
        this->updateLogUI();
    });
}

void AppLog::onViewAppear() {
    updateLogUI();

    // 开启仿真定时器，每隔1秒产生一条测试日志
    _simTimer = lv_timer_create(sim_log_cb, 1000, NULL);
}

void AppLog::onViewDisappear() {
    if (_simTimer) {
        lv_timer_del(_simTimer);
        _simTimer = nullptr;
    }
}

void AppLog::updateLogUI() {
    const auto& logs = AppManagerLog::getLogs();
    
    // 只添加新的日志
    for (size_t i = _lastLogCount; i < logs.size(); i++) {
        const auto& log = logs[i];
        
        char timeStr[16];
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", (int)((log.timestamp/1000)/60), (int)((log.timestamp/1000)%60));
        
        const char* levelStr = "INF";
        if (log.level == AM_LOG_LEVEL_WARN) levelStr = "WRN";
        if (log.level == AM_LOG_LEVEL_ERROR) levelStr = "ERR";
        
        createLogItem(timeStr, levelStr, log.message.c_str(), log.level);
    }
    
    if (_lastLogCount != logs.size()) {
        _lastLogCount = logs.size();
        // 自动滚动到底部
        lv_obj_scroll_to_view(lv_obj_get_child(_cont, -1), LV_ANIM_ON);
    }
}

lv_obj_t* AppLog::createLogItem(const char* time, const char* level, const char* msg, int level_enum) {
    // 单条日志容器
    lv_obj_t* item = lv_obj_create(_cont);
    lv_obj_set_width(item, LV_PCT(100));
    lv_obj_set_height(item, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(item, lv_color_hex(0x1C1C1E), 0); // 深灰色背景
    lv_obj_set_style_radius(item, 8, 0); // 圆角
    lv_obj_set_style_border_width(item, 0, 0);
    lv_obj_set_style_pad_all(item, 8, 0);
    lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);

    // 颜色定义
    lv_color_t levelColor = lv_color_hex(0x32D74B); // Green
    if (level_enum == AM_LOG_LEVEL_WARN) levelColor = lv_color_hex(0xFFD60A); // Yellow
    if (level_enum == AM_LOG_LEVEL_ERROR) levelColor = lv_color_hex(0xFF453A); // Red

    // 侧边颜色条（指示等级）
    lv_obj_t* bar = lv_obj_create(item);
    lv_obj_set_size(bar, 4, 20);
    lv_obj_set_style_bg_color(bar, levelColor, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_align(bar, LV_ALIGN_LEFT_MID, 0, 0);

    // 消息内容
    lv_obj_t* label = lv_label_create(item);
    lv_label_set_text_fmt(label, "#888888 %s#  %s", time, msg); // 时间灰色，内容默认白
    lv_label_set_recolor(label, true); // 开启颜色命令支持
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_width(label, LV_PCT(90));
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 10, 0);
    
    return item;
}

void AppLog::sim_log_cb(lv_timer_t* timer) {
    static int counter = 0;
    counter++;
    if (counter % 5 == 0) {
        AM_LOG_ERROR("System overload detected! (Sim %d)", counter);
    } else if (counter % 3 == 0) {
        AM_LOG_WARN("Memory usage high (Sim %d)", counter);
    } else {
        AM_LOG_INFO("Heartbeat tick %d", counter);
    }
}