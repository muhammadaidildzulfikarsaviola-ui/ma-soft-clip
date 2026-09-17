#include "SoftClip.h"

void SoftClipDSP::prepare(double sampleRate, int samplesPerBlock, int numChannels)
{
    clippers.resize(static_cast<size_t>(numChannels));
    for (auto& clipper : clippers)
        clipper.prepare(sampleRate, samplesPerBlock);

    oversamplingEngine.prepare(sampleRate, samplesPerBlock, numChannels);
    reset();
}

void SoftClipDSP::reset()
{
    for (auto& clipper : clippers)
        clipper.reset();
    oversamplingEngine.reset();
}

void SoftClipDSP::setInputGainDb(float gainDb) noexcept
{
    inputGainDb = gainDb;
    inputGainLinear = juce::Decibels::decibelsToGain(inputGainDb);
}

void SoftClipDSP::setOutputGainDb(float gainDb) noexcept
{
    outputGainDb = gainDb;
    outputGainLinear = juce::Decibels::decibelsToGain(outputGainDb);
}

void SoftClipDSP::setThresholdDb(float thresholdDb) noexcept
{
    for (auto& clipper : clippers)
        clipper.setThresholdDb(thresholdDb);
}

void SoftClipDSP::setKneePercent(float kneePercent) noexcept
{
    for (auto& clipper : clippers)
        clipper.setKneePercent(kneePercent);
}

void SoftClipDSP::setClipStyle(ClipStyle style) noexcept
{
    for (auto& clipper : clippers)
        clipper.setClipStyle(style);
}

void SoftClipDSP::setOversamplingIndex(int index) noexcept
{
    oversamplingEngine.setFactorIndex(index);
}

void SoftClipDSP::setBypass(bool isBypassed) noexcept
{
    bypassed = isBypassed;
}

void SoftClipDSP::process(juce::AudioBuffer<float>& buffer, float& outMaxGainReductionDb) noexcept
{
    outMaxGainReductionDb = 0.0f;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // 1. Apply Input Gain
    if (std::abs(inputGainLinear - 1.0f) > 0.0001f)
    {
        buffer.applyGain(inputGainLinear);
    }

    if (bypassed)
    {
        // Apply Output Gain & Exit early if bypassed
        if (std::abs(outputGainLinear - 1.0f) > 0.0001f)
            buffer.applyGain(outputGainLinear);
        return;
    }

    // 2. Oversampling Up
    juce::dsp::AudioBlock<float> block(buffer);
// 2. Oversampling Up
    juce::dsp::AudioBlock<float> block(buffer);
    auto oversampledBlock = oversamplingEngine.processSamplesUp(block);

    const int oversampledNumSamples = static_cast<int>(oversampledBlock.getNumSamples());
    float maxGrInBlock = 0.0f;

    // 3. Process Clipping per channel
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* channelData = oversampledBlock.getChannelPointer(static_cast<size_t>(ch));
        auto& clipper = clippers[static_cast<size_t>(ch)];

        for (int s = 0; s < oversampledNumSamples; ++s)
        {
            float grSampleDb = 0.0f;
            channelData[s] = clipper.processSample(channelData[s], grSampleDb);
            if (grSampleDb > maxGrInBlock)
                maxGrInBlock = grSampleDb;
        }
    }

    outMaxGainReductionDb = maxGrInBlock;

    // 4. Oversampling Down
    oversamplingEngine.processSamplesDown(block);

    // 5. Apply Output Gain
    if (std::abs(outputGainLinear - 1.0f) > 0.0001f)
    {
        buffer.applyGain(outputGainLinear);
    }
}