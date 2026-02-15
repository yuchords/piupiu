#include "AppBase.h"

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