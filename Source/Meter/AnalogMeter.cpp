#include "AnalogMeter.h"

namespace Meter
{
    AnalogMeter::AnalogMeter() {}

    void AnalogMeter::setGrValueDb(float grDb)
    {
        currentGrDb = std::abs(grDb);
        // Ballistic smoothing untuk jarum VU analog
        needlePosition += (currentGrDb - needlePosition) * 0.25f;
        repaint();
    }

    void AnalogMeter::paint(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();

        // Outer Bezel Frame Metal
        g.setColour(juce::Colour(0xff222426));
        g.fillRoundedRectangle(bounds, 6.0f);
        g.setColour(juce::Colour(0xff707378));
        g.drawRoundedRectangle(bounds, 6.0f, 2.0f);

        // Dial Face (Cream Analog Faceplate)
        auto faceBounds = bounds.reduced(8.0f);
        g.setColour(juce::Colour(0xfff2edd9));
        g.fillRoundedRectangle(faceBounds, 4.0f);

        // Scale & Text (Menggunakan juce::Justification::centred)
        g.setColour(juce::Colour(0xff1a1b1d));
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawText("dB", faceBounds.removeFromBottom(30.0f), juce::Justification::centred);
        g.drawText("GAIN REDUCTION", faceBounds.removeFromBottom(20.0f), juce::Justification::centred);

        // Needle Pivot Arc
        float pivotX = bounds.getCentreX();
        float pivotY = bounds.getBottom() + 40.0f;
        float needleLength = bounds.getHeight() * 0.85f;

        // Map 0 to 20 dB GR ke Sudut Jarum (-0.6 rad sampai +0.6 rad)
        float mappedGr = juce::jlimit(0.0f, 20.0f, needlePosition);
        float angle = juce::jmap(mappedGr, 0.0f, 20.0f, 0.55f, -0.55f);

        // Draw Needle
        float endX = pivotX + needleLength * std::sin(angle);
        float endY = pivotY - needleLength * std::cos(angle);

        g.setColour(juce::Colours::black);
        g.drawLine(pivotX, pivotY, endX, endY, 2.0f);

        // Screw Cap Pivot
        g.setColour(juce::Colour(0xff3a3d40));
        g.fillEllipse(pivotX - 8.0f, pivotY - 8.0f, 16.0f, 16.0f);
    }
}