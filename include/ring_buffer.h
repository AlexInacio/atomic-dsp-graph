#pragma once
#include <array>
#include <atomic>
#include <cstddef>

template<typename T, size_t Size>
class LockFreeRingBuffer {
private:
    std::array<T, Size> buffer;
    std::atomic<size_t> writePos{0};
    std::atomic<size_t> readPos{0};
    
public:
    bool push(const T& item) {
        size_t currentWrite = writePos.load(std::memory_order_relaxed);
        size_t nextWrite = (currentWrite + 1) % Size;
        
        if (nextWrite == readPos.load(std::memory_order_acquire)) {
            return false; // Buffer cheio
        }
        
        buffer[currentWrite] = item;
        writePos.store(nextWrite, std::memory_order_release);
        return true;
    }
    
    bool pop(T& item) {
        size_t currentRead = readPos.load(std::memory_order_relaxed);
        
        if (currentRead == writePos.load(std::memory_order_acquire)) {
            return false; // Buffer vazio
        }
        
        item = buffer[currentRead];
        readPos.store((currentRead + 1) % Size, std::memory_order_release);
        return true;
    }
    
    bool isEmpty() const {
        return readPos.load(std::memory_order_relaxed) == writePos.load(std::memory_order_relaxed);
    }
};