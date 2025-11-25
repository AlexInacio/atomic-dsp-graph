#pragma once
#include <vector>
#include <fstream>
#include <cstring>
#include <cstdint>

struct WavHeader {
    char riff[4]; uint32_t fileSize; char wave[4];
    char fmt[4]; uint32_t fmtSize; uint16_t audioFormat; uint16_t numChannels;
    uint32_t sampleRate; uint32_t byteRate; uint16_t blockAlign; uint16_t bitsPerSample;
    char data[4]; uint32_t dataSize;
};

class WavReader {
public:
    static bool read(const char* filename, std::vector<float>& samples, uint32_t& sr, uint16_t& ch) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) return false;
        
        WavHeader h;
        file.read((char*)&h, sizeof(h));
        
        if (strncmp(h.riff, "RIFF", 4) != 0) return false;
        
        sr = h.sampleRate;
        ch = h.numChannels;
        size_t count = h.dataSize / (h.bitsPerSample / 8);
        samples.resize(count);
        
        if (h.bitsPerSample == 16) {
            std::vector<int16_t> raw(count);
            file.read((char*)raw.data(), h.dataSize);
            for(size_t i=0; i<count; ++i) samples[i] = raw[i] / 32768.0f;
        }
        return true;
    }

    static bool write(const char* filename, const std::vector<float>& samples, uint32_t sr, uint16_t ch) {
        std::ofstream file(filename, std::ios::binary);
        if (!file) return false;
        
        WavHeader h;
        memcpy(h.riff, "RIFF", 4); memcpy(h.wave, "WAVE", 4);
        memcpy(h.fmt, "fmt ", 4); memcpy(h.data, "data", 4);
        
        h.fmtSize = 16; h.audioFormat = 1; h.numChannels = ch;
        h.sampleRate = sr; h.bitsPerSample = 16;
        h.blockAlign = ch * 2; h.byteRate = sr * h.blockAlign;
        h.dataSize = samples.size() * 2; h.fileSize = 36 + h.dataSize;
        
        file.write((char*)&h, sizeof(h));
        for (float s : samples) {
            int16_t v = static_cast<int16_t>(s * 32767.0f);
            file.write((char*)&v, sizeof(int16_t));
        }
        return true;
    }
};