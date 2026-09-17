#include "OverSamplingEngine.h"

void OversamplingEngine::prepare(double sampleRate, int samplesPerBlock, int numChannels) {}
void OversamplingEngine::reset() {}
void OversamplingEngine::setFactorIndex(int factorIndex) {}

juce::dsp::AudioBlock<float> OversamplingEngine::processSamplesUp(juce::dsp::AudioBlock<float>& block)
{
    return block;
}

void OversamplingEngine::processSamplesDown(juce::dsp::AudioBlock<float>& block) {}

float OversamplingEngine::getLatencyInSamples() const
{
    return 0.0f;
}