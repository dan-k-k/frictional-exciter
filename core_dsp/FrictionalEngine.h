// core_dsp/FrictionalEngine.h
#include <vector>
#include <cmath>
#include <iostream>

// Consts
const double PI = 3.14159265358979323846;

// ---------------------------------------------------------
// 1. A Single Resonant Mode (A Two-Pole Filter)
// ---------------------------------------------------------
class ModalFilter {
private:
    float a1, a2, b0;
    float z1, z2; // State variables (delay lines)
    float sampleRate;

public:
    ModalFilter(float sr) : sampleRate(sr), z1(0.0f), z2(0.0f) {}

    // Tune the specific mode
    void setParameters(float frequency, float decayTime, float gain) {
        // Calculate filter coefficients for a resonating bandpass
        float w = 2.0f * PI * frequency / sampleRate;
        float r = exp(-1.0f / (decayTime * sampleRate)); // Pole radius based on decay time

        a1 = -2.0f * r * cos(w);
        a2 = r * r;
        
        // Normalize gain so different modes sit well together
        b0 = gain * (1.0f - r); 
    }

    // Process a single sample of the incoming audio (the "strike")
    float processSample(float input) {
        float output = b0 * input - a1 * z1 - a2 * z2;
        
        // Update states
        z2 = z1;
        z1 = output;
        
        return output;
    }

    void reset() {
        z1 = 0.0f;
        z2 = 0.0f;
    }
};

// ---------------------------------------------------------
// 2. The Full Resonator Bank (The "Material")
// ---------------------------------------------------------
class FrictionalEngine {
private:
    std::vector<ModalFilter> modes;
    float sampleRate;
    
    // --- UPDATED: Exciter Envelope Follower ---
    float noiseLevel;
    float currentEnv; // Tracks the actual incoming volume continuously
    float envAttack, envRelease; // How fast it reacts to incoming audio
    
    // --- UPDATED: State Variable Filter (SVF) for Tonal Noise ---
    float svfF, svfQ; 
    float svfLow, svfHigh, svfBand;
    float noiseMixMode; // 0.0 = Lowpass (Thud), 1.0 = Bandpass (Click/Pitch), 2.0 = Highpass (Scrape)

public:
    FrictionalEngine(float sr, int numModes) : 
        sampleRate(sr), currentEnv(0.0f), svfLow(0.0f), svfHigh(0.0f), svfBand(0.0f) {
        
        for (int i = 0; i < numModes; ++i) {
            modes.emplace_back(sampleRate);
        }
        
        // Default envelope speeds (e.g., 1ms attack, 50ms release)
        envAttack = exp(-1.0f / (0.001f * sampleRate));
        envRelease = exp(-1.0f / (0.050f * sampleRate));
    }

    // --- The upgraded AI parameter receiver ---
    void setExciterParameters(float level, float filterFreq, float resonanceQ, float modeMix) {
        noiseLevel = level;
        
        // Calculate Chamberlin SVF coefficients
        // filterFreq is capped to avoid blowing up the SVF (needs to be < sr/4 roughly)
        float clampedFreq = std::min(filterFreq, sampleRate / 4.0f);
        svfF = 2.0f * sin(PI * clampedFreq / sampleRate);
        svfQ = 1.0f / std::max(0.1f, resonanceQ); // resonanceQ from 0.1 (wide) to 50.0 (ringing pitch)
        
        noiseMixMode = modeMix;
    }

    // This is where your AI / randomization scripts will feed data
    void setMaterialParameters(const std::vector<float>& freqs, 
                               const std::vector<float>& decays, 
                               const std::vector<float>& gains) {
        
        if (freqs.size() != modes.size()) return; // Safety check

        for (size_t i = 0; i < modes.size(); ++i) {
            modes[i].setParameters(freqs[i], decays[i], gains[i]);
        }
    }

    // Process a full buffer (Used for offline dataset generation AND real-time plugin)
    void processBuffer(const float* inputBuffer, float* outputBuffer, int numSamples) {
        for (int n = 0; n < numSamples; ++n) {
            float incomingAudio = inputBuffer[n];
            
            // 1. True Envelope Follower (Tracks the sustain of the incoming note)
            float absIn = std::abs(incomingAudio);
            if (absIn > currentEnv) {
                currentEnv = absIn + envAttack * (currentEnv - absIn);
            } else {
                currentEnv = absIn + envRelease * (currentEnv - absIn);
            }

            // 2. Generate pure chaos
            float whiteNoise = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            
            // 3. Apply the Chamberlin SVF (The Magic Filter)
            svfLow = svfLow + svfF * svfBand;
            svfHigh = whiteNoise - svfLow - svfQ * svfBand;
            svfBand = svfBand + svfF * svfHigh;

            // 4. Blend the filter types based on AI parameter (modeMix)
            // This allows seamless morphing between Thuds, Clicks, and Scrapes
            float shapedNoise = 0.0f;
            if (noiseMixMode < 1.0f) {
                // Morph from Lowpass to Bandpass
                shapedNoise = svfLow * (1.0f - noiseMixMode) + svfBand * noiseMixMode;
            } else {
                // Morph from Bandpass to Highpass
                shapedNoise = svfBand * (2.0f - noiseMixMode) + svfHigh * (noiseMixMode - 1.0f);
            }

            // 5. Apply the envelope and mix with incoming audio
            float physicalFriction = shapedNoise * currentEnv * noiseLevel;
            float combinedExciter = incomingAudio + physicalFriction;

            // 6. Resonate
            float outSample = 0.0f;
            for (auto& mode : modes) {
                outSample += mode.processSample(combinedExciter);
            }
            
            // Basic hard clip to save your ears during testing!
            if (outSample > 1.0f) outSample = 1.0f;
            if (outSample < -1.0f) outSample = -1.0f;
            
            outputBuffer[n] = outSample; 
        }
    }
};

