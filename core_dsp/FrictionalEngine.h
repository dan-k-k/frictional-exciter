// core_dsp/FrictionalEngine.h
#pragma once

#include <vector>
#include <cmath>
#include <memory>

// Include our modular DSP blocks
// (We will create the actual math for these next!)
#include "InputAnalyser.h"
#include "A_Modal.h"
#include "B_SlipStick.h"
#include "C_Granular.h"
#include "D_Noise.h"

class FrictionalEngine 
{
public:
    FrictionalEngine();
    ~FrictionalEngine() = default;

    // --- Core Audio Lifecycle ---
    void prepare(double sampleRate, int samplesPerBlock);
    
    // --- Parameter Receivers (From the JUCE UI) ---
    void setParameters(float pitch, float decay, float inharm, float pressure);

    // --- Main Audio Thread ---
    void process(float* channelData, int numSamples, int channel);

private:
    double currentSampleRate = 44100.0;
    
    // Max channels for a standard stereo plugin
    static constexpr int MAX_CHANNELS = 2; 

    // --- The Heuristic Brain ---
    InputAnalyser analyzer[MAX_CHANNELS];

    // --- The 4 Physical Generators ---
    A_Modal      A[MAX_CHANNELS];
    B_SlipStick  B[MAX_CHANNELS];
    C_Granular   C[MAX_CHANNELS];
    D_Noise      D[MAX_CHANNELS];

    // --- Internal Safety ---
    // A zero-latency mathematical soft-clipper to catch rogue physics spikes
    float softClip(float input);
};

