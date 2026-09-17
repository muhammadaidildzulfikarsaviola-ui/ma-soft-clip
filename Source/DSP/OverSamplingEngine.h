#pragma once

#include <juce_dsp/juce_dsp.h> 

class OversamplingEngine
{
public:
    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void reset();
    void setFactorIndex(int factorIndex);
    
    juce::dsp::AudioBlock<float> processSamplesUp(juce::dsp::AudioBlock<float>& block);
    void processSamplesDown(juce::dsp::AudioBlock<float>& block);
    
    float getLatencyInSamples() const;
};