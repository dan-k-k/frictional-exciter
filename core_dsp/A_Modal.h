// core_dsp/A_Modal.h
#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

namespace DSPMath {
    constexpr double PI = 3.14159265358979323846;
}

// ---------------------------------------------------------
// 1. The Single Resonator (The 2-Pole Bandpass)
// ---------------------------------------------------------
class ModalFilter {
private:
    float a1 = 0.0f, a2 = 0.0f, b0 = 0.0f;
    float z1 = 0.0f, z2 = 0.0f; 
    double sampleRate = 44100.0;

public:
    ModalFilter() = default;

    void prepare(double sr) {
        sampleRate = sr;
        reset();
    }

    void setParameters(float frequency, float decayTime, float gain) {
        // Clamp frequency to prevent the filter from blowing up (Nyquist limit)
        float clampedFreq = std::min(frequency, static_cast<float>(sampleRate * 0.49));
        
        float w = 2.0f * DSPMath::PI * clampedFreq / sampleRate;
        float r = std::exp(-1.0f / (decayTime * sampleRate)); // Pole radius

        a1 = -2.0f * r * std::cos(w);
        a2 = r * r;
        
        // Normalize the gain so loud modes don't clip the engine
        b0 = gain * (1.0f - r); 
    }

    // Process a single sample
    inline float process(float input) {
        float output = b0 * input - a1 * z1 - a2 * z2;
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
// 2. Pillar A: The Material Bank
// ---------------------------------------------------------
class A_Modal {
private:
    // 8 modes is usually the sweet spot for prototyping. 
    // Enough to sound rich, light enough on CPU.
    static constexpr int NUM_MODES = 8; 
    std::vector<ModalFilter> modes;
    double sampleRate = 44100.0;

public:
    A_Modal() {
        modes.resize(NUM_MODES);
    }

    void prepare(double sr) {
        sampleRate = sr;
        for (auto& mode : modes) {
            mode.prepare(sr);
        }
    }

    // The core physical modeling translation function
    void setResonatorParams(float basePitch, float baseDecay, float inharmonicity) {
        for (int i = 0; i < NUM_MODES; ++i) {
            // 1. Calculate the harmonic multiplier for this mode (1, 2, 3, 4...)
            float modeIndex = static_cast<float>(i + 1);
            
            // 2. Apply the Inharmonicity stretch.
            // If inharm = 0, this is just modeIndex.
            // If inharm = 1, this curves upwards wildly (stiff metal).
            float stretchFactor = 1.0f + (inharmonicity * 0.5f * (modeIndex * modeIndex - 1.0f));
            float modeFreq = basePitch * modeIndex * stretchFactor;

            // 3. High frequencies should decay faster than low frequencies naturally
            // We scale the decay down as the mode index gets higher
            float dampingRatio = 1.0f / std::sqrt(modeIndex);
            float modeDecay = baseDecay * dampingRatio;

            // 4. Gain taper (lower modes are louder, higher modes are quieter)
            float modeGain = 1.0f / modeIndex;

            modes[i].setParameters(modeFreq, modeDecay, modeGain);
        }
    }

    // Process the exact sample passed in by the Motherboard
    inline float process(float input) {
        float output = 0.0f;
        for (auto& mode : modes) {
            output += mode.process(input);
        }
        return output;
    }
};

