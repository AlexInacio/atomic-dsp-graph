#include <iostream>
#include <vector>
#include "memory_arena.h"
#include "audio_nodes.h"
#include "wav_io.h"

class AudioEngine {
    MemoryArena arena;
    std::vector<float> buffer1, buffer2, outputBuffer;
    uint32_t sampleRate = 44100;
    uint16_t channels = 2;

public:
    AudioEngine(size_t arenaSize) : arena(arenaSize) {}

    bool loadFiles(const char* f1, const char* f2) {
        if (!WavReader::read(f1, buffer1, sampleRate, channels)) return false;
        if (!WavReader::read(f2, buffer2, sampleRate, channels)) return false;
        outputBuffer.resize(std::max(buffer1.size(), buffer2.size()));
        return true;
    }

    void process(float g1, float g2) {
        // Alocação Zero: Objetos criados na stack ou pré-alocados
        GainNode gainNode1(g1);
        GainNode gainNode2(g2);
        MixerNode mixer;

        AudioBuffer b1(buffer1.data(), buffer1.size());
        AudioBuffer b2(buffer2.data(), buffer2.size());
        AudioBuffer out(outputBuffer.data(), outputBuffer.size());

        gainNode1.process(b1);
        gainNode2.process(b2);
        mixer.mix(b1, b2, out);
        
        std::cout << "[Engine] Processamento concluído. Memória de Arena usada: " << arena.used() << " bytes.\n";
    }

    bool save(const char* out) {
        return WavReader::write(out, outputBuffer, sampleRate, channels);
    }
};

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: ./mixer_app <in1.wav> <in2.wav> <out.wav>\n";
        return 1;
    }

    AudioEngine engine(1024 * 1024 * 10); // 10MB Arena
    if (!engine.loadFiles(argv[1], argv[2])) {
        std::cerr << "Falha ao carregar arquivos.\n";
        return 1;
    }

    engine.process(0.8f, 0.6f);
    engine.save(argv[3]);
    return 0;
}