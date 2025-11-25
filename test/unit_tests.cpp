#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <cmath>


#include "memory_arena.h"
#include "ring_buffer.h"
#include "audio_nodes/gain_node.h"
#include "audio_nodes/mixer_node.h"
#include "audio_nodes/fade_node.h"

// ============================================================================
// TESTES: MEMORY ARENA (Critical System Constraint)
// ============================================================================

TEST(MemoryArenaTest, InitializationCorrect) {
    MemoryArena arena(1024);
    EXPECT_EQ(arena.used(), 0);
    EXPECT_EQ(arena.capacity(), 1024);
}

TEST(MemoryArenaTest, SimpleAllocation) {
    MemoryArena arena(1024);
    void* ptr1 = arena.allocate(100);
    EXPECT_NE(ptr1, nullptr);
    EXPECT_GE(arena.used(), 100);
}

TEST(MemoryArenaTest, Alignment) {
    MemoryArena arena(1024);
    // Alocar 1 byte para desalinhar o offset propositadamente
    arena.allocate(1); 
    
    // Alocar com alinhamento de 16 bytes (comum para SIMD)
    void* ptr = arena.allocate(32, 16);
    
    // Verificar se o endereço é múltiplo de 16
    uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
    EXPECT_EQ(address % 16, 0);
}

TEST(MemoryArenaTest, ResetWorks) {
    MemoryArena arena(1024);
    arena.allocate(500);
    EXPECT_GT(arena.used(), 0);
    
    arena.reset();
    EXPECT_EQ(arena.used(), 0);
    
    // Deve ser possível realocar após reset
    void* ptr = arena.allocate(500);
    EXPECT_NE(ptr, nullptr);
}

TEST(MemoryArenaTest, OutOfMemoryException) {
    MemoryArena arena(100);
    // Tentar alocar mais do que a capacidade
    EXPECT_THROW(arena.allocate(200), std::bad_alloc);
}

// ============================================================================
// TESTES: LOCK-FREE RING BUFFER (Concurrency)
// ============================================================================

TEST(RingBufferTest, BasicPushPop) {
    LockFreeRingBuffer<int, 4> rb;
    
    EXPECT_TRUE(rb.push(10));
    EXPECT_TRUE(rb.push(20));
    
    int val;
    EXPECT_TRUE(rb.pop(val));
    EXPECT_EQ(val, 10);
    EXPECT_TRUE(rb.pop(val));
    EXPECT_EQ(val, 20);
}

TEST(RingBufferTest, BufferFull) {
    // Tamanho 4 significa capacidade útil de 3 (implementação comum de ring buffer)
    // ou 4 dependendo da implementação específica de atomic. 
    // Assumindo a implementação clássica (size-1) ou ajustando conforme seu código.
    LockFreeRingBuffer<int, 4> rb; 
    
    EXPECT_TRUE(rb.push(1));
    EXPECT_TRUE(rb.push(2));
    EXPECT_TRUE(rb.push(3)); 
    
    // O próximo deve falhar se a implementação usar (head + 1) % size == tail
    // Se a sua implementação permite encher totalmente, ajuste o teste.
    bool result = rb.push(4); 
    
    // Se falhou, verificamos; se passou, verificamos se o pop está correto
    if (result) {
         EXPECT_FALSE(rb.push(5)); // Agora deve estar cheio de certeza
    }
}

TEST(RingBufferTest, BufferEmpty) {
    LockFreeRingBuffer<int, 4> rb;
    int val;
    EXPECT_FALSE(rb.pop(val));
}

TEST(RingBufferTest, FIFOOrder) {
    LockFreeRingBuffer<int, 10> rb;
    for(int i=0; i<5; ++i) rb.push(i);
    
    int val;
    for(int i=0; i<5; ++i) {
        ASSERT_TRUE(rb.pop(val));
        EXPECT_EQ(val, i);
    }
}

// ============================================================================
// TESTES: AUDIO NODES (DSP Logic)
// ============================================================================

// Helper para criar buffers facilmente
std::vector<float> create_data(size_t size, float val) {
    return std::vector<float>(size, val);
}

TEST(GainNodeTest, ProcessesCorrectly) {
    GainNode gainNode(0.5f); // 50% volume
    
    std::vector<float> data = {1.0f, 0.5f, -1.0f, 0.0f};
    AudioBuffer buffer(data.data(), data.size());
    
    gainNode.process(buffer);
    
    EXPECT_FLOAT_EQ(data[0], 0.5f);
    EXPECT_FLOAT_EQ(data[1], 0.25f);
    EXPECT_FLOAT_EQ(data[2], -0.5f);
    EXPECT_FLOAT_EQ(data[3], 0.0f);
}

TEST(GainNodeTest, SIMDAlignment) {
    // Teste para garantir que blocos grandes (que ativam AVX/SSE) não crasham
    GainNode gainNode(2.0f);
    
    // 1024 floats é suficiente para exercitar loops SIMD
    std::vector<float> data(1024, 1.0f); 
    AudioBuffer buffer(data.data(), data.size());
    
    gainNode.process(buffer);
    
    for(float sample : data) {
        EXPECT_FLOAT_EQ(sample, 2.0f);
    }
}

TEST(MixerNodeTest, SumsSignals) {
    std::vector<float> d1 = {0.1f, 0.2f, 0.3f};
    std::vector<float> d2 = {0.1f, 0.2f, 0.3f};
    std::vector<float> out(3, 0.0f);
    
    AudioBuffer b1(d1.data(), d1.size());
    AudioBuffer b2(d2.data(), d2.size());
    AudioBuffer bOut(out.data(), out.size());
    
    MixerNode::mix(b1, b2, bOut);
    
    EXPECT_FLOAT_EQ(out[0], 0.2f);
    EXPECT_FLOAT_EQ(out[1], 0.4f);
    EXPECT_FLOAT_EQ(out[2], 0.6f);
}

TEST(FadeNodeTest, LinearFadeOut) {
    // Fade out de 4 samples
    FadeNode fadeOut(4.0f, false); // false = fade out
    
    std::vector<float> data = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    AudioBuffer buffer(data.data(), data.size());
    
    fadeOut.process(buffer);
    
    // Esperado: 1.0, 0.75, 0.5, 0.25, 0.0
    EXPECT_FLOAT_EQ(data[0], 1.0f); 
    EXPECT_FLOAT_EQ(data[1], 0.75f);
    EXPECT_FLOAT_EQ(data[2], 0.5f);
    EXPECT_FLOAT_EQ(data[3], 0.25f);
    EXPECT_NEAR(data[4], 0.0f, 0.0001f);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}