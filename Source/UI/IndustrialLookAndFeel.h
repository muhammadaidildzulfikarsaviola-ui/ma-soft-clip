#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace IndustrialStyle
{
    // Palette Warna Hardware Industrial
    const juce::Colour metalBackground { 0xffc8cacc };
    const juce::Colour metalPanel      { 0xffd2d4d6 };
    const juce::Colour panelBorder     { 0xff8a8d91 };
    const juce::Colour textDark        { 0xff1a1b1d };
    const juce::Colour knobBody        { 0xff222426 };
    const juce::Colour knobCap         { 0xff151617 };
    const juce::Colour indicatorOrange { 0xffff9500 };
    const juce::Colour amberLedOn      { 0xffffab00 };
    const juce::Colour amberLedOff     { 0xff3a2b10 };
    const juce::Colour screwHead       { 0xff707378 };

    // Helper untuk menggambar sekrup panel hardware
    inline void drawScrew(juce::Graphics& g, float x, float y, float diameter = 10.0f, float angleRadian = 0.785f)
    {
        juce::Graphics::ScopedSaveState saveState(g);
        const float radius = diameter * 0.5f;

        // Shadow & Body Sekrup
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillEllipse(x - radius + 1.0f, y - radius + 1.0f, diameter, diameter);

        g.setGradientFill(juce::ColourGradient(
            screwHead.brighter(0.2f), x - radius, y - radius,
            screwHead.darker(0.4f), x + radius, y + radius, false));
        g.fillEllipse(x - radius, y - radius, diameter, diameter);

        // Bezel Outer Ring
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawEllipse(x - radius, y - radius, diameter, diameter, 0.8f);

        // Slot Obeng Cross/Flat
        g.setColour(juce::Colour(0xff121314));
        juce::Line<float> slotLine(
            x - (radius * 0.55f) * std::cos(angleRadian),
            y - (radius * 0.55f) * std::sin(angleRadian),
            x + (radius * 0.55f) * std::cos(angleRadian),
            y + (radius * 0.55f) * std::sin(angleRadian)
        );
        g.drawLine(slotLine, 1.2f);
    }
}

class IndustrialLookAndFeel : public juce::LookAndFeel_V4
{
public:
    IndustrialLookAndFeel()
    {
        setColour(juce::Slider::rotarySliderFillColourId, IndustrialStyle::indicatorOrange);
        setColour(juce::Slider::thumbColourId, juce::Colours::white);
        setColour(juce::Label::textColourId, IndustrialStyle::textDark);
    }

    // Custom Knob Metal Industrial dengan Garis Penunjuk Putih
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override
    {
        const float radius = (juce::jmin(width, height) * 0.5f) - 6.0f;
        const float centreX = (float)x + (float)width * 0.5f;
        const float centreY = (float)y + (float)height * 0.5f;
        const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Drop Shadow Knob
        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.fillEllipse(centreX - radius + 2.0f, centreY - radius + 3.0f, radius * 2.0f, radius * 2.0f);

        // Outer Bezel Ring (Metal Dark)
        g.setGradientFill(juce::ColourGradient(
            juce::Colour(0xff55585c), centreX - radius, centreY - radius,
            juce::Colour(0xff1a1b1d), centreX + radius, centreY + radius, false));
        g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

        // Knob Body (Brushed Dark Metal)
        const float innerRadius = radius - 3.0f;
        g.setGradientFill(juce::ColourGradient(
            IndustrialStyle::knobBody.brighter(0.15f), centreX - innerRadius, centreY - innerRadius,
            IndustrialStyle::knobCap, centreX + innerRadius, centreY + innerRadius, false));
        g.fillEllipse(centreX - innerRadius, centreY - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

        // Inner Circle Cap
        const float capRadius = innerRadius * 0.75f;
        g.setGradientFill(juce::ColourGradient(
            juce::Colour(0xff2a2c2e), centreX - capRadius, centreY - capRadius,
            juce::Colour(0xff101112), centreX + capRadius, centreY + capRadius, false));
        g.fillEllipse(centreX - capRadius, centreY - capRadius, capRadius * 2.0f, capRadius * 2.0f);
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.drawEllipse(centreX - capRadius, centreY - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.0f);

        // Indicator Line (White Line dari center cap ke outer rim)
        juce::Path pointer;
        const float pointerLength = innerRadius - 2.0f;
        pointer.addRectangle(-1.2f, -pointerLength, 2.4f, pointerLength - (capRadius * 0.3f));
        
        g.setColour(juce::Colours::white);
        g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    }

    // Custom Toggle Switch (Vintage Toggle Lever)
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        const float toggleW = 34.0f;
        const float toggleH = 18.0f;
        const float x = bounds.getX() + 2.0f;
        const float y = bounds.getCentreY() - (toggleH * 0.5f);

        // Base Track/Slot
        g.setColour(juce::Colour(0xff121314));
        g.fillRoundedRectangle(x, y, toggleW, toggleH, toggleH * 0.5f);
        g.setColour(juce::Colour(0xff606368));
        g.drawRoundedRectangle(x, y, toggleW, toggleH, toggleH * 0.5f, 1.0f);

        // Lever Position
        const float knobR = (toggleH - 4.0f) * 0.5f;
        const float togglePos = button.getToggleState() ? (x + toggleW - knobR - 3.0f) : (x + knobR + 3.0f);

        // Chrome/Metal Lever Handle
        g.setGradientFill(juce::ColourGradient(
            juce::Colours::white, togglePos - knobR, y + 2.0f,
            juce::Colour(0xff707378), togglePos + knobR, y + toggleH - 2.0f, false));
        g.fillEllipse(togglePos - knobR, y + 2.0f, knobR * 2.0f, knobR * 2.0f);

        // Label Text
        g.setColour(IndustrialStyle::textDark);
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawText(button.getButtonText(), bounds.removeFromRight(bounds.getWidth() - toggleW - 8.0f),
                   juce::Justification::centredLeft, true);
    }
};