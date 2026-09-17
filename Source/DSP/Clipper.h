#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

enum class ClipStyle
{
    Soft = 0,
    Medium,
    Hard
};

class Clipper
{
public:
    Clipper() = default;

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    void setThresholdDb(float thresholdDb) noexcept;
    void setKneePercent(float kneePercent) noexcept;
    void setClipStyle(ClipStyle style) noexcept;

    // Saturated sample output & calculation of gain reduction per sample
    float processSample(float inputSample, float& outGainReductionDb) noexcept;

private:
    float thresholdDb { 0.0f };
    float thresholdLinear { 1.0f };
    float kneePercent { 50.0f };
    float kneeWidthDb { 3.0f }; // Transisi dB knee
    ClipStyle style { ClipStyle::Soft };

    float processSoft(float x, float thresh) noexcept;
    float processMedium(float x, float thresh) noexcept;
    float processHard(float x, float thresh) noexcept;
};