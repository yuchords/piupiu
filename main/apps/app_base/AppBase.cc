#include "AppBase.h"
#include <cstring>

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
}

void AppBase::setCustomCacheEnable(bool en) {
    setCustomAutoCacheEnable(false);
    _reqEnableCache = en;
}

void AppBase::setCustomAutoCacheEnable(bool en) {
    _reqDisableAutoCache = !en;
}


bool AppBase::stashExtract(void* ptr, uint32_t size) {
    if(_stash.ptr == nullptr) {
        return false;
    }

    if(_stash.size != size) {
        return false;
        // TODO: WARN LOG
    }

    memcpy(ptr, _stash.ptr, _stash.size);
    lv_mem_free(_stash.ptr);
    _stash.ptr = nullptr;

    return true;

}
