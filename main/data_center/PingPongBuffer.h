#pragma once

#include <cstdint>

class PingPongBuffer {
public:
    PingPongBuffer();
    ~PingPongBuffer() = default;

    void init(void* buf0, void* buf1);
    bool getReadBuf(void** pReadBuf);
    void setReadDone();
    void getWriteBuf(void** pWriteBuf);
    void setWriteDone();
    void* getBaseBuffer();

private:
    void* _buffer[2];
    volatile uint8_t _writeIndex;
    volatile uint8_t _readIndex;
    volatile bool _readAvailable[2];
};
