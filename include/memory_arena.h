#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <new>

class MemoryArena {
private:
    std::vector<uint8_t> buffer;
    size_t offset = 0;
    
public:
    explicit MemoryArena(size_t size) : buffer(size) {
        std::cout << "[Arena] Alocados " << size / 1024 << " KB\n";
    }
    
    // Desabilita cópia para evitar erros de ponteiro
    MemoryArena(const MemoryArena&) = delete;
    MemoryArena& operator=(const MemoryArena&) = delete;

    void* allocate(size_t size, size_t alignment = 16) {
        size_t padding = (alignment - (offset % alignment)) % alignment;
        
        if (offset + padding + size > buffer.size()) {
            throw std::bad_alloc();
        }

        offset += padding;
        void* ptr = buffer.data() + offset;
        offset += size;
        return ptr;
    }
    
    template<typename T, typename... Args>
    T* emplace(Args&&... args) {
        void* mem = allocate(sizeof(T), alignof(T));
        return new(mem) T(std::forward<Args>(args)...);
    }
    
    void reset() { offset = 0; }
    size_t used() const { return offset; }
    size_t capacity() const { return buffer.size(); }
};