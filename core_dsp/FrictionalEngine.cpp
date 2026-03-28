// core_dsp/FrictionalEngine.cpp
#include "FrictionalEngine.h"

FrictionalEngine::FrictionalEngine()
{
    // Constructor logic (if any) goes here
}

void FrictionalEngine::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // Initialize all components with the DAW's sample rate for accurate math
    for (int ch = 0; ch < MAX_CHANNELS; ++ch) 
    {
        analyzer[ch].prepare(sampleRate);
        A[ch].prepare(sampleRate);
        B[ch].prepare(sampleRate);
        C[ch].prepare(sampleRate);
        D[ch].prepare(sampleRate);
    }
}

void FrictionalEngine::setParameters(float pitch, float decay, float inharm, float pressure)
{
    // Pass the raw parameters down to the specific pillars that need them
    for (int ch = 0; ch < MAX_CHANNELS; ++ch) 
    {
        A[ch].setResonatorParams(pitch, decay, inharm);
        B[ch].setFrictionParams(pressure);
        // C & D parameters will go here later
    }
}

void FrictionalEngine::process(float* channelData, int numSamples, int channel)
{
    // Safety check to prevent out-of-bounds memory access if a DAW sends surround sound
    if (channel >= MAX_CHANNELS) return; 

    for (int n = 0; n < numSamples; ++n) 
    {
        float inputSample = channelData[n];

        // 1. ANALYZE: Is this a click (1.0) or a sustained note (0.0)?
        float transientScore = analyzer[channel].getTransientScore(inputSample);

        // 2. GENERATE: Create the raw physical textures
        float frictionTexture = B[channel].process(inputSample); // Needs input audio
        float granularBurst   = C[channel].process(transientScore); // Fires on transient
        float noiseWash       = D[channel].process(); // Continuous

        // 3. ROUTE: Blend the exciter based on the heuristic brain
        // If transient -> use granular/burst. If tonal -> use slip-stick friction.
        float totalExcitation = (frictionTexture * (1.0f - transientScore)) + 
                                (granularBurst * transientScore);

        // 4. RESONATE: Feed the blended kinetic energy into the tuned material
        float resonatedBody = A[channel].process(totalExcitation);

        // 5. OUTPUT: Mix the body with the top-end noise and clip it safely
        float finalOutput = resonatedBody + (noiseWash * 0.05f); // Noise kept very quiet
        
        channelData[n] = softClip(finalOutput);
    }
}

float FrictionalEngine::softClip(float input)
{
    // A fast, zero-latency cubic soft-clipper 
    // If signal pushes past -1 to 1, it bends it back gracefully instead of harsh digital clipping
    if (input <= -1.0f) return -0.666666667f;
    if (input >= 1.0f) return 0.666666667f;
    return input - (input * input * input) * 0.333333333f;
}

