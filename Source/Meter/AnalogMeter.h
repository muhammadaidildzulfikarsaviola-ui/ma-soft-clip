#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace Meter
{
    class AnalogMeter : public juce::Component
    {
    public:
        AnalogMeter();
        ~AnalogMeter() override = default;

        void setGrValueDb(float grDb);
        void paint(juce::Graphics& g) override;

    private:
        float currentGrDb { 0.0f };
        float needlePosition { 0.0f }; // Smoothed for ballistics

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogMeter)
    };
}