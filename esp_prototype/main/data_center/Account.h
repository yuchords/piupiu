#pragma once

#include <cstdint>
#include <vector>
#include "PingPongBuffer.h"
#include <lvgl.h>

class DataCenter;

class Account {
public:
    enum EventCode {
        EVENT_NONE,
        EVENT_PUB_PUBLISH, // Publisher posted information
        EVENT_SUB_PULL,    // Subscriber data pull request
        EVENT_NOTIFY,      // Subscribers send notifications to publishers
        EVENT_TIMER,       // Timed event
        _EVENT_LAST
    };

    enum ResCode {
        RES_OK                  =  0,
        RES_UNKNOWN             = -1,
        RES_SIZE_MISMATCH       = -2,
        RES_UNSUPPORTED_REQUEST = -3,
        RES_NO_CALLBACK         = -4,
        RES_NO_CACHE            = -5,
        RES_NO_COMMITTED        = -6,
        RES_NOT_FOUND           = -7,
        RES_PARAM_ERROR         = -8
    };

    struct EventParam {
        EventCode event;   // Event type
        Account* tran;     // Pointer to sender
        Account* recv;     // Pointer to receiver
        void* data_p;      // Pointer to data
        uint32_t size;     // The length of the data
    };

    typedef int(*EventCallback)(Account* account, EventParam* param);
    typedef std::vector<Account*> AccountVector;

public:
    Account(const char* id, DataCenter* center, uint32_t bufSize = 0, void* userData = nullptr);
    ~Account();

    Account* subscribe(const char* pubID);
    bool unsubscribe(const char* pubID);
    bool commit(const void* data_p, uint32_t size);
    int publish();
    int pull(const char* pubID, void* data_p, uint32_t size);
    int pull(Account* pub, void* data_p, uint32_t size);
    int notify(const char* pubID, const void* data_p, uint32_t size);
    int notify(Account* pub, const void* data_p, uint32_t size);
    
    void setEventCallback(EventCallback callback);
    void setTimerPeriod(uint32_t period);
    void setTimerEnable(bool en);
    
    size_t getPublishersSize() const;
    size_t getSubscribersSize() const;

public:
    const char* _id;
    DataCenter* _center;
    void* _userData;

    AccountVector _publishers;
    AccountVector _subscribers;

    struct {
        EventCallback eventCallback;
        lv_timer_t* timer;
        PingPongBuffer bufferManager;
        uint32_t bufferSize;
    } _priv;

private:
    static void timerCallbackHandler(lv_timer_t* task);
};
