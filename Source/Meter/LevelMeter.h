#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace Meter
{
    class LevelMeter : public juce::Component
    {
    public:
        LevelMeter() = default;
        ~LevelMeter() override = default;

        void setLevels(float peakLevel, float rmsLevel)
        {
            peak = juce::jlimit(0.0f, 1.0f, peakLevel);
            rms  = juce::jlimit(0.0f, 1.0f, rmsLevel);
            repaint();
        }

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();

            // Background Slot Meter
            g.setColour(juce::Colour(0xff121314));
            g.fillRect(bounds);
            g.setColour(juce::Colour(0xff606368));
            g.drawRect(bounds, 1.0f);

            const int totalSegments = 24;
            const float segHeight = (bounds.getHeight() - 4.0f) / (float)totalSegments;

            // Segmented LED Meter
            for (int i = 0; i < totalSegments; ++i)
            {
                float segY = bounds.getBottom() - (i + 1) * segHeight - 2.0f;
                float fillRatio = (float)(i + 1) / (float)totalSegments;

                juce::Colour segColor;
                if (fillRatio > 0.85f)
                    segColor = juce::Colours::red;
                else if (fillRatio > 0.70f)
                    segColor = juce::Colour(0xffff9500); // Amber
                else
                    segColor = juce::Colour(0xff00e676); // Green

                if (peak >= fillRatio)
                {
                    g.setColour(segColor);
                    g.fillRect(bounds.getX() + 2.0f, segY + 1.0f, bounds.getWidth() - 4.0f, segHeight - 1.0f);
                }
                else
                {
                    g.setColour(segColor.withAlpha(0.15f));
                    g.fillRect(bounds.getX() + 2.0f, segY + 1.0f, bounds.getWidth() - 4.0f, segHeight - 1.0f);
                }
            }
        }

    private:
        float peak { 0.0f };
        float rms  { 0.0f };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
    };
}