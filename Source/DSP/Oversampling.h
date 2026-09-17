#pragma once

#include <juce_dsp/juce_dsp.h>
#include <memory>

class OversamplingEngine
{
public:
    OversamplingEngine() = default;

    void prepare(double sampleRate, int maxBlockSize, int numChannels);
    void reset();

    void setFactorIndex(int index); // 0 = 1x, 1 = 2x, 2 = 4x, 3 = 8x
    int getFactorIndex() const noexcept { return currentFactorIndex; }

    juce::dsp::AudioBlock<float> processSamplesUp(juce::dsp::AudioBlock<float>& inputBlock);
    void processSamplesDown(juce::dsp::AudioBlock<float>& outputBlock);

    float getLatencyInSamples() const noexcept;

private:
    double sampleRate { 44100.0 };
    int maxBlockSize { 512 };
    int numChannels { 2 };
    int currentFactorIndex { 0 };

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler2x;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler4x;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler8x;
};