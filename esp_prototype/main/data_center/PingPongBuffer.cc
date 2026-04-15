#include "PingPongBuffer.h"
#include <cstring>

PingPongBuffer::PingPongBuffer() {
    std::memset(this, 0, sizeof(PingPongBuffer));
}

void PingPongBuffer::init(void* buf0, void* buf1) {
    _buffer[0] = buf0;
    _buffer[1] = buf1;
    _writeIndex = 0;
    _readIndex = 0;
    _readAvailable[0] = false;
    _readAvailable[1] = false;
}

bool PingPongBuffer::getReadBuf(void** pReadBuf) {
    if (_readAvailable[0]) {
        _readIndex = 0;
    } else if (_readAvailable[1]) {
        _readIndex = 1;
    } else {
        return false;
    }
    *pReadBuf = _buffer[_readIndex];
    return true;
}

void PingPongBuffer::setReadDone() {
    _readAvailable[_readIndex] = false;
}

void PingPongBuffer::getWriteBuf(void** pWriteBuf) {
    if (_writeIndex == _readIndex) {
        _writeIndex = !_readIndex;
    }
    *pWriteBuf = _buffer[_writeIndex];
}

void PingPongBuffer::setWriteDone() {
    _readAvailable[_writeIndex] = true;
    _writeIndex = !_writeIndex;
}

void* PingPongBuffer::getBaseBuffer() {
    return _buffer[0];
}
