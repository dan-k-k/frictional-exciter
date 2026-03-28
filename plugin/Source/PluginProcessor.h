// plugin/Source/PluginProcessor.h
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
// This points to your pure DSP class outside the JUCE wrapper
#include "FrictionalEngine.h" 

class FrictionalExciterProcessor : public juce::AudioProcessor
{
public:
    FrictionalExciterProcessor();
    ~FrictionalExciterProcessor() override;

    // --- Core Audio Engine ---
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // --- The Generic UI Cheat Code ---
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    // --- Plugin Identity Boilderplate ---
    const juce::String getName() const override { return "Frictional Exciter"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int index) override {}
    const juce::String getProgramName (int index) override { return {}; }
    void changeProgramName (int index, const juce::String& newName) override {}

    // --- State Saving/Loading ---
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // --- Parameter Management ---
    juce::AudioProcessorValueTreeState treeState;

private:
    // This static function builds the parameter list before the plugin loads
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Your raw DSP engine (Pillars A, B, C, D)
    FrictionalEngine engine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrictionalExciterProcessor)
};

