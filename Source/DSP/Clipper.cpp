#include "Clipper.h"
#include <cmath>

void Clipper::prepare(double /*sampleRate*/, int /*samplesPerBlock*/)
{
    reset();
}

void Clipper::reset() {}

void Clipper::setThresholdDb(float newThresholdDb) noexcept
{
    thresholdDb = newThresholdDb;
    thresholdLinear = juce::Decibels::decibelsToGain(thresholdDb);
}

void Clipper::setKneePercent(float newKneePercent) noexcept
{
    kneePercent = juce::jlimit(0.0f, 100.0f, newKneePercent);
    // Scale 0-100% to 0-6 dB knee width
    kneeWidthDb = (kneePercent / 100.0f) * 6.0f;
}

void Clipper::setClipStyle(ClipStyle newStyle) noexcept
{
    style = newStyle;
}

float Clipper::processSample(float x, float& outGainReductionDb) noexcept
{
    const float absX = std::abs(x);
    if (absX < 0.000001f)
    {
        outGainReductionDb = 0.0f;
        return x;
    }

    const float signX = (x > 0.0f) ? 1.0f : -1.0f;
    float y = absX;

    const float threshDb = thresholdDb;
    const float inDb = juce::Decibels::gainToDecibels(absX);

    // Apply Soft Knee interpolation near threshold
    if (kneeWidthDb > 0.001f && inDb > (threshDb - kneeWidthDb * 0.5f))
    {
        const float lowerBound = threshDb - kneeWidthDb * 0.5f;
        const float upperBound = threshDb + kneeWidthDb * 0.5f;

        if (inDb < upperBound)
        {
            // Quadratic knee blend
            const float t = (inDb - lowerBound) / kneeWidthDb;
            const float blendedThreshLinear = juce::Decibels::decibelsToGain(threshDb + (t * t * kneeWidthDb * 0.25f));
            
            switch (style)
            {
                case ClipStyle::Soft:   y = processSoft(absX, blendedThreshLinear); break;
                case ClipStyle::Medium: y = processMedium(absX, blendedThreshLinear); break;
                case ClipStyle::Hard:   y = processHard(absX, blendedThreshLinear); break;
            }
        }
        else
        {
            switch (style)
            {
                case ClipStyle::Soft:   y = processSoft(absX, thresholdLinear); break;
                case ClipStyle::Medium: y = processMedium(absX, thresholdLinear); break;
                case ClipStyle::Hard:   y = processHard(absX, thresholdLinear); break;
            }
        }
    }
    else if (absX > thresholdLinear)
    {
        switch (style)
        {
            case ClipStyle::Soft:   y = processSoft(absX, thresholdLinear); break;
            case ClipStyle::Medium: y = processMedium(absX, thresholdLinear); break;
            case ClipStyle::Hard:   y = processHard(absX, thresholdLinear); break;
        }
    }

    // Gain Reduction dalam dB
    const float outDb = juce::Decibels::gainToDecibels(y + 1e-9f);
    outGainReductionDb = std::max(0.0f, inDb - outDb);

    return y * signX;
}

float Clipper::processSoft(float x, float thresh) noexcept
{
    // Smooth Hyperbolic Tangent curve above threshold
    const float over = x - thresh;
    return thresh + std::tanh(over / thresh) * thresh * 0.5f;
}

float Clipper::processMedium(float x, float thresh) noexcept
{
    // Cubic Soft Clipper (Algebraic transition)
    const float over = x - thresh;
    const float scaled = over / (thresh * 1.5f);
    if (scaled >= 1.0f)
        return thresh * 1.333f;
    return thresh + (over - (std::pow(scaled, 3.0f) * over / 3.0f));
}

float Clipper::processHard(float x, float thresh) noexcept
{
    // Brickwall Ceiling at threshold
    return std::min(x, thresh);
}