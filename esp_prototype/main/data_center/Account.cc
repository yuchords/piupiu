#include "Account.h"
#include "DataCenter.h"
#include "DataCenterLog.h"
#include <cstring>
#include <algorithm>

#define ACCOUNT_DISCARD_READ_DATA 1

Account::Account(const char* id, DataCenter* center, uint32_t bufSize, void* userData) {
    std::memset(&_priv, 0, sizeof(_priv));

    _id = id;
    _center = center;
    _userData = userData;

    if (bufSize != 0) {
        uint8_t* buffer = (uint8_t*)lv_mem_alloc(bufSize * sizeof(uint8_t) * 2);

        if (!buffer) {
            DC_LOG_ERROR("Account[%s] buffer malloc failed!", _id);
            return;
        }

        std::memset(buffer, 0, bufSize * sizeof(uint8_t) * 2);

        uint8_t* buf0 = buffer;
        uint8_t* buf1 = buffer + bufSize;

        _priv.bufferManager.init(buf0, buf1);
        DC_LOG_INFO("Account[%s] cached %lu x2 bytes", _id, static_cast<unsigned long>(bufSize));
        _priv.bufferSize = bufSize;
    }

    _center->addAccount(this);

    DC_LOG_INFO("Account[%s] created", _id);
}

Account::~Account() {
    DC_LOG_INFO("Account[%s] deleting...", _id);

    if (_priv.bufferSize) {
        lv_mem_free(_priv.bufferManager.getBaseBuffer());
    }

    if (_priv.timer) {
        lv_timer_del(_priv.timer);
        DC_LOG_INFO("Account[%s] task deleted", _id);
    }

    for (auto iter : _subscribers) {
        iter->unsubscribe(_id);
        DC_LOG_INFO("sub[%s] unsubscribed pub[%s]", iter->_id, _id);
    }

    for (auto iter : _publishers) {
        _center->remove(&iter->_subscribers, this);
        DC_LOG_INFO("pub[%s] removed sub[%s]", iter->_id, _id);
    }

    _center->removeAccount(this);
    DC_LOG_INFO("Account[%s] deleted", _id);
}

Account* Account::subscribe(const char* pubID) {
    if (std::strcmp(pubID, _id) == 0) {
        DC_LOG_ERROR("Account[%s] try to subscribe to itself", _id);
        return nullptr;
    }

    Account* pub = _center->find(&_publishers, pubID);
    if (pub != nullptr) {
        DC_LOG_ERROR("Multi subscribe pub[%s]", pubID);
        return nullptr;
    }

    pub = _center->searchAccount(pubID);
    if (pub == nullptr) {
        DC_LOG_ERROR("pub[%s] was not found", pubID);
        return nullptr;
    }

    _publishers.push_back(pub);
    pub->_subscribers.push_back(this);

    DC_LOG_INFO("sub[%s] subscribed pub[%s]", _id, pubID);
    return pub;
}

bool Account::unsubscribe(const char* pubID) {
    Account* pub = _center->find(&_publishers, pubID);
    if (pub == nullptr) {
        DC_LOG_WARN("sub[%s] was not subscribe pub[%s]", _id, pubID);
        return false;
    }

    _center->remove(&_publishers, pub);
    _center->remove(&pub->_subscribers, this);

    return true;
}

bool Account::commit(const void* data_p, uint32_t size) {
    if (!size || size != _priv.bufferSize) {
        DC_LOG_ERROR("pub[%s] has not cache or size mismatch", _id);
        return false;
    }

    void* wBuf;
    _priv.bufferManager.getWriteBuf(&wBuf);

    std::memcpy(wBuf, data_p, size);

    _priv.bufferManager.setWriteDone();

    DC_LOG_INFO("pub[%s] commit data(0x%p)[%lu] >> data(0x%p)[%lu] done",
                _id, data_p, static_cast<unsigned long>(size), wBuf, static_cast<unsigned long>(size));

    return true;
}

int Account::publish() {
    int retval = RES_UNKNOWN;

    if (_priv.bufferSize == 0) {
        DC_LOG_ERROR("pub[%s] has not cache", _id);
        return RES_NO_CACHE;
    }

    void* rBuf;
    if (!_priv.bufferManager.getReadBuf(&rBuf)) {
        DC_LOG_WARN("pub[%s] data was not commit", _id);
        return RES_NO_COMMITTED;
    }

    EventParam param;
    param.event = EVENT_PUB_PUBLISH;
    param.tran = this;
    param.recv = nullptr;
    param.data_p = rBuf;
    param.size = _priv.bufferSize;

    for (auto sub : _subscribers) {
        EventCallback callback = sub->_priv.eventCallback;

        DC_LOG_INFO("pub[%s] publish >> data(0x%p)[%lu] >> sub[%s]...",
                    _id, param.data_p, static_cast<unsigned long>(param.size), sub->_id);

        if (callback != nullptr) {
            param.recv = sub;
            int ret = callback(sub, &param);
            DC_LOG_INFO("publish done: %d", ret);
            retval = ret;
        } else {
            DC_LOG_INFO("sub[%s] not register callback", sub->_id);
        }
    }

#if ACCOUNT_DISCARD_READ_DATA
    _priv.bufferManager.setReadDone();
#endif

    return retval;
}

int Account::pull(const char* pubID, void* data_p, uint32_t size) {
    Account* pub = _center->find(&_publishers, pubID);
    if (pub == nullptr) {
        DC_LOG_ERROR("sub[%s] was not subscribe pub[%s]", _id, pubID);
        return RES_NOT_FOUND;
    }
    return pull(pub, data_p, size);
}

int Account::pull(Account* pub, void* data_p, uint32_t size) {
    int retval = RES_UNKNOWN;

    if (pub == nullptr) {
        return RES_NOT_FOUND;
    }

    DC_LOG_INFO("sub[%s] pull << data(0x%p)[%lu] << pub[%s] ...",
                _id, data_p, static_cast<unsigned long>(size), pub->_id);

    EventCallback callback = pub->_priv.eventCallback;
    if (callback != nullptr) {
        EventParam param;
        param.event = EVENT_SUB_PULL;
        param.tran = this;
        param.recv = pub;
        param.data_p = data_p;
        param.size = size;

        int ret = callback(pub, &param);

        DC_LOG_INFO("pull done: %d", ret);
        retval = ret;
    } else {
        DC_LOG_INFO("pub[%s] not registed pull callback, read commit cache...", pub->_id);

        if (pub->_priv.bufferSize == size) {
            void* rBuf;
            if (pub->_priv.bufferManager.getReadBuf(&rBuf)) {
                std::memcpy(data_p, rBuf, size);
#if ACCOUNT_DISCARD_READ_DATA
                pub->_priv.bufferManager.setReadDone();
#endif
                DC_LOG_INFO("read done");
                retval = 0;
            } else {
                DC_LOG_WARN("pub[%s] data was not commit!", pub->_id);
            }
        } else {
            DC_LOG_ERROR("Data size pub[%s]:%lu != sub[%s]:%lu",
                         pub->_id, static_cast<unsigned long>(pub->_priv.bufferSize), _id, static_cast<unsigned long>(size));
        }
    }

    return retval;
}

int Account::notify(const char* pubID, const void* data_p, uint32_t size) {
    Account* pub = _center->find(&_publishers, pubID);
    if (pub == nullptr) {
        DC_LOG_ERROR("sub[%s] was not subscribe pub[%s]", _id, pubID);
        return RES_NOT_FOUND;
    }
    return notify(pub, data_p, size);
}

int Account::notify(Account* pub, const void* data_p, uint32_t size) {
    int retval = RES_UNKNOWN;

    if (pub == nullptr) {
        return RES_NOT_FOUND;
    }

    DC_LOG_INFO("sub[%s] notify >> data(0x%p)[%lu] >> pub[%s] ...",
                _id, data_p, static_cast<unsigned long>(size), pub->_id);

    EventCallback callback = pub->_priv.eventCallback;
    if (callback != nullptr) {
        EventParam param;
        param.event = EVENT_NOTIFY;
        param.tran = this;
        param.recv = pub;
        param.data_p = (void*)data_p;
        param.size = size;

        int ret = callback(pub, &param);

        DC_LOG_INFO("send done: %d", ret);
        retval = ret;
    } else {
        DC_LOG_WARN("pub[%s] not register callback", pub->_id);
        retval = RES_NO_CALLBACK;
    }

    return retval;
}

void Account::setEventCallback(EventCallback callback) {
    _priv.eventCallback = callback;
}

void Account::timerCallbackHandler(lv_timer_t* timer) {
    Account* instance = (Account*)(timer->user_data);
    EventCallback callback = instance->_priv.eventCallback;
    if (callback) {
        EventParam param;
        param.event = EVENT_TIMER;
        param.tran = instance;
        param.recv = instance;
        param.data_p = nullptr;
        param.size = 0;

        callback(instance, &param);
    }
}

void Account::setTimerPeriod(uint32_t period) {
    if (_priv.timer) {
        lv_timer_del(_priv.timer);
        _priv.timer = nullptr;
    }

    if (period == 0) {
        return;
    }

    _priv.timer = lv_timer_create(timerCallbackHandler, period, this);
}

void Account::setTimerEnable(bool en) {
    lv_timer_t* timer = _priv.timer;

    if (timer == nullptr) {
        return;
    }

    en ? lv_timer_resume(timer) : lv_timer_pause(timer);
}

size_t Account::getPublishersSize() const {
    return _publishers.size();
}

size_t Account::getSubscribersSize() const {
    return _subscribers.size();
}
