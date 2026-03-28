// plugin/Source/PluginProcessor.cpp
#include "PluginProcessor.h"

// --- 1. Construct & Initialize ---
FrictionalExciterProcessor::FrictionalExciterProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
#endif
       // Initialize the parameter tree, linking it to the layout we define below
       treeState(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

FrictionalExciterProcessor::~FrictionalExciterProcessor() {}

// --- 2. Define the Prototype Parameters ---
juce::AudioProcessorValueTreeState::ParameterLayout FrictionalExciterProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Pillar A: Modal Resonator Parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"MODAL_PITCH", 1}, "Base Pitch (Hz)", 20.0f, 2000.0f, 440.0f));
        
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"MODAL_DECAY", 1}, "Decay Time (s)", 0.1f, 10.0f, 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"MODAL_INHARMONICITY", 1}, "Inharmonicity (Wood -> Metal)", 0.0f, 1.0f, 0.0f));

    // Pillar B: Friction Exciter Parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"FRICTION_PRESSURE", 1}, "Bow Pressure", 0.0f, 1.0f, 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"OUTPUT_GAIN", 1}, "Output Gain (dB)", -24.0f, 12.0f, 0.0f));

    return { params.begin(), params.end() };
}

// --- 3. The UI Cheat Code ---
juce::AudioProcessorEditor* FrictionalExciterProcessor::createEditor()
{
    // This tells JUCE to automatically generate a basic slider UI based on the parameters above.
    return new juce::GenericAudioProcessorEditor(*this);
}

bool FrictionalExciterProcessor::hasEditor() const { return true; }

// --- 4. DSP Preparation & Processing ---
void FrictionalExciterProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Pass sample rate to your raw DSP engine so it can calculate filter coefficients
    engine.prepare(sampleRate, samplesPerBlock);
}

void FrictionalExciterProcessor::releaseResources() { }

void FrictionalExciterProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // 1. Fetch current parameter values from the UI
    float pitch = treeState.getRawParameterValue("MODAL_PITCH")->load();
    float decay = treeState.getRawParameterValue("MODAL_DECAY")->load();
    float inharm = treeState.getRawParameterValue("MODAL_INHARMONICITY")->load();
    float pressure = treeState.getRawParameterValue("FRICTION_PRESSURE")->load();
    
    // 2. Update the engine's internal state
    engine.setParameters(pitch, decay, inharm, pressure);

    // 3. Process the audio!
    // (We pass the raw channel pointers to keep FrictionalEngine independent of juce::AudioBuffer)
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);
        int numSamples = buffer.getNumSamples();
        
        engine.process(channelData, numSamples, channel);
    }
}

// --- 5. State Management (Saving/Loading your DAW session) ---
void FrictionalExciterProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = treeState.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void FrictionalExciterProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (treeState.state.getType()))
            treeState.replaceState (juce::ValueTree::fromXml (*xmlState));
}

// This JUCE macro instantiates the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FrictionalExciterProcessor();
}

