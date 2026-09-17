#include "Oversampling.h"

void OversamplingEngine::prepare(double sr, int maxBlock, int numChans)
{
    sampleRate = sr;
    maxBlockSize = maxBlock;
    numChannels = numChans;

    oversampler2x = std::make_unique<juce::dsp::Oversampling<float>>(
        numChannels, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true);
    oversampler4x = std::make_unique<juce::dsp::Oversampling<float>>(
        numChannels, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true);
    oversampler8x = std::make_unique<juce::dsp::Oversampling<float>>(
        numChannels, 3, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true);

    oversampler2x->initProcessing(static_cast<size_t>(maxBlockSize));
    oversampler4x->initProcessing(static_cast<size_t>(maxBlockSize));
    oversampler8x->initProcessing(static_cast<size_t>(maxBlockSize));

    reset();
}

void OversamplingEngine::reset()
{
    if (oversampler2x) oversampler2x->reset();
    if (oversampler4x) oversampler4x->reset();
    if (oversampler8x) oversampler8x->reset();
}

void OversamplingEngine::setFactorIndex(int index)
{
    currentFactorIndex = juce::jlimit(0, 3, index);
}

juce::dsp::AudioBlock<float> OversamplingEngine::processSamplesUp(juce::dsp::AudioBlock<float>& inputBlock)
{
    switch (currentFactorIndex)
    {
        case 1: return oversampler2x->processSamplesUp(inputBlock);
        case 2: return oversampler4x->processSamplesUp(inputBlock);
        case 3: return oversampler8x->processSamplesUp(inputBlock);
        default: return inputBlock;
    }
}

void OversamplingEngine::processSamplesDown(juce::dsp::AudioBlock<float>& outputBlock)
{
    switch (currentFactorIndex)
    {
        case 1: oversampler2x->processSamplesDown(outputBlock); break;
        case 2: oversampler4x->processSamplesDown(outputBlock); break;
        case 3: oversampler8x->processSamplesDown(outputBlock); break;
        default: break;
    }
}

float OversamplingEngine::getLatencyInSamples() const noexcept
{
    switch (currentFactorIndex)
    {
        case 1: return oversampler2x->getLatencyInSamples();
        case 2: return oversampler4x->getLatencyInSamples();
        case 3: return oversampler8x->getLatencyInSamples();
        default: return 0.0f;
    }
}