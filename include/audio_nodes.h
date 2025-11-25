#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <immintrin.h>

struct AudioBuffer {
    float* data;
    size_t size;
    size_t channels;
    
    AudioBuffer(float* d, size_t s, size_t c = 1) 
        : data(d), size(s), channels(c) {}
};

// Base CRTP
template<typename Derived>
class AudioNode {
public:
    void process(AudioBuffer& buffer) {
        static_cast<Derived*>(this)->processImpl(buffer);
    }
};

class GainNode : public AudioNode<GainNode> {
    float gain;
public:
    explicit GainNode(float g = 1.0f) : gain(g) {}
    
    void processImpl(AudioBuffer& buffer) {
        #ifdef __AVX__
        size_t simdSize = (buffer.size / 8) * 8;
        __m256 gainVec = _mm256_set1_ps(gain);
        
        for (size_t i = 0; i < simdSize; i += 8) {
            __m256 samples = _mm256_loadu_ps(&buffer.data[i]);
            samples = _mm256_mul_ps(samples, gainVec);
            _mm256_storeu_ps(&buffer.data[i], samples);
        }
        for (size_t i = simdSize; i < buffer.size; ++i) buffer.data[i] *= gain;
        #else
        for (size_t i = 0; i < buffer.size; ++i) buffer.data[i] *= gain;
        #endif
    }
};

class FadeNode : public AudioNode<FadeNode> {
    float duration;
    float currentSample = 0;
    bool fadeIn;
public:
    FadeNode(float durationSamples, bool in = true) 
        : duration(durationSamples), fadeIn(in) {}
    
    void processImpl(AudioBuffer& buffer) {
        for (size_t i = 0; i < buffer.size; ++i) {
            float factor = fadeIn ? 
                std::min(currentSample / duration, 1.0f) : 
                std::max(1.0f - (currentSample / duration), 0.0f);
            
            buffer.data[i] *= factor;
            currentSample++;
        }
    }
    void reset() { currentSample = 0; }
};

class MixerNode : public AudioNode<MixerNode> {
public:
    void processImpl(AudioBuffer&) {} // Placeholder para interface
    
    static void mix(const AudioBuffer& in1, const AudioBuffer& in2, AudioBuffer& out) {
        size_t len = std::min({in1.size, in2.size, out.size});
        
        #ifdef __AVX__
        size_t simdSize = (len / 8) * 8;
        for (size_t i = 0; i < simdSize; i += 8) {
            __m256 a = _mm256_loadu_ps(&in1.data[i]);
            __m256 b = _mm256_loadu_ps(&in2.data[i]);
            _mm256_storeu_ps(&out.data[i], _mm256_add_ps(a, b));
        }
        for (size_t i = simdSize; i < len; ++i) out.data[i] = in1.data[i] + in2.data[i];
        #else
        for (size_t i = 0; i < len; ++i) out.data[i] = in1.data[i] + in2.data[i];
        #endif
    }
};