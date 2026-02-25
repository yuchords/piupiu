#include "AppBase.h"
#include "AppLog.h"
#include <cstring>

/**
 * @brief 构造函数
 * @note 初始化应用状态
 */
AppBase::AppBase()
    : _root(nullptr),
      _manager(nullptr),
      _name(nullptr),
      _id(0),
      _userData(nullptr),
      _reqEnableCache(false),
      _reqDisableAutoCache(false),
      _isDisableAutoCache(false),
      _isCached(false) {
    _stash.ptr = nullptr;
    _stash.size = 0;
    _state = APP_STATE_IDLE;
    _anim.isEnter = false;
    _anim.isBusy = false;
    _anim.attr.type = 0;
    _anim.attr.time = 0;
    _anim.attr.path = nullptr;
}

/**
 * @brief 设置自定义缓存启用状态
 * @param en 是否启用缓存
 */
void AppBase::setCustomCacheEnable(bool en) {
    setCustomAutoCacheEnable(false);
    _reqEnableCache = en;
}

/**
 * @brief 设置自定义自动缓存启用状态
 * @param en 是否启用自动缓存
 */
void AppBase::setCustomAutoCacheEnable(bool en) {
    _reqDisableAutoCache = !en;
}

/**
 * @brief 从缓存中提取数据
 * @param ptr 目标指针
 * @param size 数据大小
 * @return 是否成功提取
 */
bool AppBase::stashExtract(void* ptr, uint32_t size) {
    if(_stash.ptr == nullptr) {
        AM_LOG_WARN("stashExtract: stash is empty");
        return false;
    }

    if(_stash.size != size) {
        AM_LOG_WARN("stashExtract: stash size mismatch");
        return false;
    }

    memcpy(ptr, _stash.ptr, _stash.size);
    lv_mem_free(_stash.ptr);
    _stash.ptr = nullptr;

    return true;

}

/**
 * @brief 设置自定义加载动画类型
 * @param animType 动画类型
 * @param time 动画时间
 * @param path 动画路径回调函数
 */
void AppBase::setCustomLoadAnimType(uint8_t animType, uint16_t time, lv_anim_path_cb_t path) {
    _anim.attr.type = animType;
    _anim.attr.time = time;
    _anim.attr.path = path;
}
