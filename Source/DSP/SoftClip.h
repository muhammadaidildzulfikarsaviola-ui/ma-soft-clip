#pragma once

#include "Clipper.h"
#include "Oversampling.h"
#include <vector>

enum class ProcessingMode
{
    Stereo = 0,
    MidSide,
    Multiband
};

enum class SignalSelect
{
    Input = 0,
    GR,
    Output
};

class SoftClipDSP
{

public:
    SoftClipDSP() = default;

    void setMode(ProcessingMode newMode);
    void setSignalSelect(SignalSelect newSelect);
    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void reset();

    void process(juce::AudioBuffer<float>& buffer, float& outMaxGainReductionDb) noexcept;

    void setInputGainDb(float gainDb) noexcept;
    void setOutputGainDb(float gainDb) noexcept;
    void setThresholdDb(float thresholdDb) noexcept;
    void setKneePercent(float kneePercent) noexcept;
    void setClipStyle(ClipStyle style) noexcept;
    void setOversamplingIndex(int index) noexcept;
    void setBypass(bool isBypassed) noexcept;

    float getLatencyInSamples() const noexcept { return oversamplingEngine.getLatencyInSamples(); }

private:
    float inputGainDb { 0.0f };
    float inputGainLinear { 1.0f };
    float outputGainDb { 0.0f };
    float outputGainLinear { 1.0f };
    bool bypassed { false };

    std::vector<Clipper> clippers;
    OversamplingEngine oversamplingEngine;
};