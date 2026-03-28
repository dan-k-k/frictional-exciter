// core_dsp/main.cpp
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "FrictionalEngine.h"

// ==============================================================================
// LEAN .WAV I/O HELPERS (16-bit Mono PCM)
// ==============================================================================
bool readMonoWav(const std::string& filename, std::vector<float>& audioData, int& sampleRate) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;

    char buffer[4];
    file.seekg(24); // Skip to sample rate
    file.read(buffer, 4);
    sampleRate = *reinterpret_cast<int*>(buffer);

    file.seekg(40); // Skip to data chunk size
    file.read(buffer, 4);
    int dataSize = *reinterpret_cast<int*>(buffer);
    int numSamples = dataSize / 2; // 16-bit = 2 bytes per sample

    audioData.resize(numSamples);
    int16_t sample;
    for (int i = 0; i < numSamples; ++i) {
        file.read(reinterpret_cast<char*>(&sample), 2);
        audioData[i] = sample / 32768.0f; // Convert to -1.0 to 1.0 float
    }
    return true;
}

bool writeMonoWav(const std::string& filename, const std::vector<float>& audioData, int sampleRate) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) return false;

    int numSamples = audioData.size();
    int byteRate = sampleRate * 2;
    int dataSize = numSamples * 2;
    int chunkSize = 36 + dataSize;

    // Write WAV Header
    file << "RIFF";
    file.write(reinterpret_cast<const char*>(&chunkSize), 4);
    file << "WAVEfmt ";
    int subchunk1Size = 16; short audioFormat = 1; short numChannels = 1; short bitsPerSample = 16; short blockAlign = 2;
    file.write(reinterpret_cast<const char*>(&subchunk1Size), 4);
    file.write(reinterpret_cast<const char*>(&audioFormat), 2);
    file.write(reinterpret_cast<const char*>(&numChannels), 2);
    file.write(reinterpret_cast<const char*>(&sampleRate), 4);
    file.write(reinterpret_cast<const char*>(&byteRate), 4);
    file.write(reinterpret_cast<const char*>(&blockAlign), 2);
    file.write(reinterpret_cast<const char*>(&bitsPerSample), 2);
    file << "data";
    file.write(reinterpret_cast<const char*>(&dataSize), 4);

    // Write Data
    for (float sample : audioData) {
        // Hard clip to prevent wrap-around distortion
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        int16_t intSample = static_cast<int16_t>(sample * 32767.0f);
        file.write(reinterpret_cast<const char*>(&intSample), 2);
    }
    return true;
}

// ==============================================================================
// THE MAIN APP
// ==============================================================================
int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: ./render_sound <input.wav> <output.wav> <params.txt>\n";
        return 1;
    }

    std::string inFile = argv[1];
    std::string outFile = argv[2];
    std::string paramsFile = argv[3];

    // 1. Read the calibration track
    std::vector<float> inputBuffer;
    int sampleRate;
    if (!readMonoWav(inFile, inputBuffer, sampleRate)) {
        std::cerr << "Failed to read input file: " << inFile << "\n";
        return 1;
    }

    // 2. Read the parameters from text file
    std::ifstream pFile(paramsFile);
    if (!pFile) {
        std::cerr << "Failed to read params file: " << paramsFile << "\n";
        return 1;
    }

    float exciterLevel, exciterFreq, exciterQ, exciterMix;
    pFile >> exciterLevel >> exciterFreq >> exciterQ >> exciterMix;

    std::vector<float> freqs, decays, gains;
    float f, d, g;
    while (pFile >> f >> d >> g) {
        freqs.push_back(f);
        decays.push_back(d);
        gains.push_back(g);
    }

    // 3. Initialize the Engine
    int numModes = freqs.size();
    FrictionalEngine engine(static_cast<float>(sampleRate), numModes);
    
    engine.setExciterParameters(exciterLevel, exciterFreq, exciterQ, exciterMix);
    engine.setMaterialParameters(freqs, decays, gains);

    // 4. Process the audio
    std::vector<float> outputBuffer(inputBuffer.size());
    engine.processBuffer(inputBuffer.data(), outputBuffer.data(), inputBuffer.size());

    // 5. Save the result
    if (!writeMonoWav(outFile, outputBuffer, sampleRate)) {
        std::cerr << "Failed to write output file: " << outFile << "\n";
        return 1;
    }

    // std::cout << "Successfully rendered " << numModes << " modes to " << outFile << "\n";
    return 0;
}

