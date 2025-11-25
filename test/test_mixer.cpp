#include <iostream>
#include <cassert>
#include "../include/memory_arena.h"
#include "../include/ring_buffer.h"

void test_ring_buffer() {
    std::cout << "[Test] Ring Buffer... ";
    LockFreeRingBuffer<int, 4> buffer;
    
    assert(buffer.push(1));
    assert(buffer.push(2));
    assert(buffer.push(3));
    // Buffer deve ter 3 itens, tamanho 4. Próximo push falha ou sucesso depende da impl (cheio = size-1 em algumas)
    // Na nossa impl, usamos atomics.
    
    int val;
    assert(buffer.pop(val) && val == 1);
    assert(buffer.pop(val) && val == 2);
    
    std::cout << "OK\n";
}

void test_memory_arena() {
    std::cout << "[Test] Memory Arena... ";
    MemoryArena arena(1024);
    
    void* p1 = arena.allocate(100);
    assert(arena.used() >= 100);
    
    void* p2 = arena.allocate(100);
    assert(p2 > p1);
    
    arena.reset();
    assert(arena.used() == 0);
    
    std::cout << "OK\n";
}

int main() {
    test_ring_buffer();
    test_memory_arena();
    std::cout << "Todos os testes passaram!\n";
    return 0;
}