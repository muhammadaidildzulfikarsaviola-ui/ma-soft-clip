#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    void setupValueLabel (juce::Label& label)
    {
        label.setFont (juce::Font (10.5f, juce::Font::bold));
        label.setColour (juce::Label::textColourId, IndustrialStyle::textDark.withAlpha (0.78f));
        label.setJustificationType (juce::Justification::centred);
        label.setInterceptsMouseClicks (false, false);
    }

    juce::String formatDb (float value)
    {
        const auto sign = value > -0.0001f ? "+" : "";
        return sign + juce::String (value, 1) + " dB";
    }
}

SoftClipAudioProcessorEditor::SoftClipAudioProcessorEditor (SoftClipAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel (&industrialLaf);
    setSize (1024, 576);

    auto setupKnob = [this] (juce::Slider& knob, juce::Label& label, const juce::String& text)
    {
        knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        knob.setRange (0.0, 1.0);
        addAndMakeVisible (knob);

        label.setText (text, juce::dontSendNotification);
        label.setFont (juce::Font (12.0f, juce::Font::bold));
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
    };

    auto setupStepSlider = [this] (juce::Slider& slider, juce::Label& label, const juce::String& text)
    {
        slider.setSliderStyle (juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible (slider);

        label.setText (text, juce::dontSendNotification);
        label.setFont (juce::Font (12.0f, juce::Font::bold));
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
    };

    setupKnob (inputGainKnob, inputGainLabel, "GAIN");
    setupKnob (outputGainKnob, outputGainLabel, "OUTPUT");
    setupKnob (thresholdKnob, thresholdLabel, "THRESHOLD");
    setupKnob (kneeKnob, kneeLabel, "KNEE");

    setupStepSlider (styleSlider, styleLabel, "CLIPPER STYLE");
    setupStepSlider (modeSlider, modeLabel, "MODE");
    setupStepSlider (oversampleSlider, oversampleLabel, "OVERSAMPLING");

    signalSelectSlider.setSliderStyle (juce::Slider::LinearVertical);
    signalSelectSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible (signalSelectSlider);
    signalSelectLabel.setText ("SIGNAL", juce::dontSendNotification);
    signalSelectLabel.setFont (juce::Font (11.0f, juce::Font::bold));
    signalSelectLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (signalSelectLabel);

    clipperBypassToggle.setButtonText ("CLIPPER");
    addAndMakeVisible (clipperBypassToggle);

    // Realtime meters
    addAndMakeVisible (inputMeterL);
    addAndMakeVisible (inputMeterR);
    addAndMakeVisible (outputMeterL);
    addAndMakeVisible (outputMeterR);
    addAndMakeVisible (analogVuMeter);

    inputReadoutLabel.setFont (juce::Font (10.5f, juce::Font::bold));
    inputReadoutLabel.setJustificationType (juce::Justification::centred);
    inputReadoutLabel.setColour (juce::Label::textColourId, IndustrialStyle::textDark);
    addAndMakeVisible (inputReadoutLabel);

    outputReadoutLabel.setFont (juce::Font (10.5f, juce::Font::bold));
    outputReadoutLabel.setJustificationType (juce::Justification::centred);
    outputReadoutLabel.setColour (juce::Label::textColourId, IndustrialStyle::textDark);
    addAndMakeVisible (outputReadoutLabel);

    // Live value labels
    setupValueLabel (inputGainValueLabel);
    setupValueLabel (outputGainValueLabel);
    setupValueLabel (thresholdValueLabel);
    setupValueLabel (kneeValueLabel);
    setupValueLabel (styleValueLabel);
    setupValueLabel (modeValueLabel);
    setupValueLabel (oversampleValueLabel);
    setupValueLabel (signalSelectValueLabel);
    setupValueLabel (clipperStateLabel);

    addAndMakeVisible (inputGainValueLabel);
    addAndMakeVisible (outputGainValueLabel);
    addAndMakeVisible (thresholdValueLabel);
    addAndMakeVisible (kneeValueLabel);
    addAndMakeVisible (styleValueLabel);
    addAndMakeVisible (modeValueLabel);
    addAndMakeVisible (oversampleValueLabel);
    addAndMakeVisible (signalSelectValueLabel);
    addAndMakeVisible (clipperStateLabel);

    // APVTS attachments
    auto& apvts = audioProcessor.getAPVTS();
    inputGainAttach.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, IDs::inputGain, inputGainKnob));
    outputGainAttach.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, IDs::outputGain, outputGainKnob));
    thresholdAttach.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, IDs::threshold, thresholdKnob));
    kneeAttach.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, IDs::knee, kneeKnob));
    styleAttach.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, IDs::clipStyle, styleSlider));
    modeAttach.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, IDs::mode, modeSlider));
    oversampleAttach.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, IDs::oversample, oversampleSlider));
    signalSelectAttach.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (apvts, IDs::signalSelect, signalSelectSlider));
    bypassAttach.reset (new juce::AudioProcessorValueTreeState::ButtonAttachment (apvts, IDs::bypass, clipperBypassToggle));

    startTimer (33);
}

SoftClipAudioProcessorEditor::~SoftClipAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void SoftClipAudioProcessorEditor::timerCallback()
{
    const float inPeakL = audioProcessor.getInputPeakL();
    const float inRmsL = audioProcessor.getInputRmsL();
    const float outPeakL = audioProcessor.getOutputPeakL();
    const float outRmsL = audioProcessor.getOutputRmsL();
    const float grDb = audioProcessor.getCurrentGrDb();

    inputMeterL.setLevels (inPeakL, inRmsL);
    inputMeterR.setLevels (audioProcessor.getInputPeakR(), audioProcessor.getInputRmsR());
    outputMeterL.setLevels (outPeakL, outRmsL);
    outputMeterR.setLevels (audioProcessor.getOutputPeakR(), audioProcessor.getOutputRmsR());
    analogVuMeter.setGrValueDb (grDb);

    const float inPeakDb = juce::Decibels::gainToDecibels (inPeakL, -60.0f);
    const float outPeakDb = juce::Decibels::gainToDecibels (outPeakL, -60.0f);
    inputReadoutLabel.setText (juce::String (inPeakDb, 1) + " dB\nLEVEL IN", juce::dontSendNotification);
    outputReadoutLabel.setText (juce::String (outPeakDb, 1) + " dB\nLEVEL OUT", juce::dontSendNotification);

    auto& apvts = audioProcessor.getAPVTS();
    const float inputGain = apvts.getRawParameterValue (IDs::inputGain)->load();
    const float outputGain = apvts.getRawParameterValue (IDs::outputGain)->load();
    const float threshold = apvts.getRawParameterValue (IDs::threshold)->load();
    const float knee = apvts.getRawParameterValue (IDs::knee)->load();
    const int clipStyle = juce::jlimit (0, 2, juce::roundToInt (apvts.getRawParameterValue (IDs::clipStyle)->load()));
    const int mode = juce::jlimit (0, 2, juce::roundToInt (apvts.getRawParameterValue (IDs::mode)->load()));
    const int oversample = juce::jlimit (0, 3, juce::roundToInt (apvts.getRawParameterValue (IDs::oversample)->load()));
    const int signalSelect = juce::jlimit (0, 2, juce::roundToInt (apvts.getRawParameterValue (IDs::signalSelect)->load()));
    const bool bypassOn = apvts.getRawParameterValue (IDs::bypass)->load() > 0.5f;

    inputGainValueLabel.setText (formatDb (inputGain), juce::dontSendNotification);
    outputGainValueLabel.setText (formatDb (outputGain), juce::dontSendNotification);
    thresholdValueLabel.setText (formatDb (threshold), juce::dontSendNotification);
    kneeValueLabel.setText (juce::String (knee, 0) + " %", juce::dontSendNotification);

    static const char* const clipStyles[] = { "SOFT", "MEDIUM", "HARD" };
    static const char* const modes[] = { "STEREO", "M/S", "MULTIBAND" };
    static const char* const oversampling[] = { "1x", "2x", "4x", "8x" };
    static const char* const signals[] = { "IN", "GR", "OUT" };

    styleValueLabel.setText (clipStyles[clipStyle], juce::dontSendNotification);
    modeValueLabel.setText (modes[mode], juce::dontSendNotification);
    oversampleValueLabel.setText (oversampling[oversample], juce::dontSendNotification);
    signalSelectValueLabel.setText (signals[signalSelect], juce::dontSendNotification);
    clipperStateLabel.setText (bypassOn ? "ON" : "OFF", juce::dontSendNotification);
}

void SoftClipAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (IndustrialStyle::metalBackground);

    g.setColour (IndustrialStyle::panelBorder);
    g.drawRect (getLocalBounds().toFloat(), 2.0f);

    g.setColour (IndustrialStyle::textDark);
    g.setFont (juce::Font (26.0f, juce::Font::bold));
    g.drawText ("SOFT CLIP", 24, 16, 200, 32, juce::Justification::left);

    g.setFont (juce::Font (11.0f, juce::Font::bold));
    g.drawText ("CLEAN LOUDER TOGETHER", 170, 24, 200, 20, juce::Justification::left);

    g.setFont (juce::Font (20.0f, juce::Font::bold));
    g.drawText ("WADIDAW", getWidth() - 200, 14, 180, 24, juce::Justification::right);
    g.setFont (juce::Font (10.0f, juce::Font::bold));
    g.drawText ("AUDIO TOOLS", getWidth() - 200, 36, 180, 16, juce::Justification::right);

    auto drawSectionBox = [&g] (juce::Rectangle<float> bounds)
    {
        g.setColour (IndustrialStyle::metalPanel);
        g.fillRect (bounds);
        g.setColour (IndustrialStyle::panelBorder);
        g.drawRect (bounds, 1.5f);

        IndustrialStyle::drawScrew (g, bounds.getX() + 8.0f, bounds.getY() + 8.0f, 9.0f, 0.5f);
        IndustrialStyle::drawScrew (g, bounds.getRight() - 8.0f, bounds.getY() + 8.0f, 9.0f, 1.2f);
        IndustrialStyle::drawScrew (g, bounds.getX() + 8.0f, bounds.getBottom() - 8.0f, 9.0f, 2.1f);
        IndustrialStyle::drawScrew (g, bounds.getRight() - 8.0f, bounds.getBottom() - 8.0f, 9.0f, 0.8f);
    };

    drawSectionBox (juce::Rectangle<float> (16.0f, 60.0f, 210.0f, 380.0f));
    drawSectionBox (juce::Rectangle<float> (236.0f, 60.0f, 540.0f, 480.0f));
    drawSectionBox (juce::Rectangle<float> (786.0f, 60.0f, 222.0f, 480.0f));
    drawSectionBox (juce::Rectangle<float> (16.0f, 450.0f, 210.0f, 90.0f));

    IndustrialStyle::drawScrew (g, 12, 12, 12.0f, 0.3f);
    IndustrialStyle::drawScrew (g, getWidth() - 12, 12, 12.0f, 1.1f);
    IndustrialStyle::drawScrew (g, 12, getHeight() - 12, 12.0f, 2.4f);
    IndustrialStyle::drawScrew (g, getWidth() - 12, getHeight() - 12, 12.0f, 1.7f);

    g.setFont (juce::Font (10.0f, juce::Font::bold));
    g.setColour (IndustrialStyle::textDark.withAlpha (0.7f));
    g.drawText ("WADIDAW AUDIO TOOLS", 24, getHeight() - 22, 200, 16, juce::Justification::left);
    g.drawText ("CLEAN LOUDER TOGETHER", getWidth() / 2 - 100, getHeight() - 22, 200, 16, juce::Justification::centred);
    g.drawText ("EST. 2025", getWidth() - 224, getHeight() - 22, 200, 16, juce::Justification::right);
}

void SoftClipAudioProcessorEditor::resized()
{
    inputMeterL.setBounds (36, 90, 16, 260);
    inputMeterR.setBounds (56, 90, 16, 260);
    outputMeterL.setBounds (150, 90, 16, 260);
    outputMeterR.setBounds (170, 90, 16, 260);

    inputReadoutLabel.setBounds (26, 355, 80, 34);
    outputReadoutLabel.setBounds (136, 355, 80, 34);

    analogVuMeter.setBounds (256, 80, 280, 170);

    thresholdLabel.setBounds (560, 68, 90, 16);
    thresholdKnob.setBounds (560, 85, 90, 90);
    thresholdValueLabel.setBounds (552, 176, 106, 18);

    kneeLabel.setBounds (670, 68, 90, 16);
    kneeKnob.setBounds (670, 85, 90, 90);
    kneeValueLabel.setBounds (662, 176, 106, 18);

    inputGainLabel.setBounds (300, 270, 110, 16);
    inputGainKnob.setBounds (300, 290, 110, 110);
    inputGainValueLabel.setBounds (292, 402, 126, 18);

    outputGainLabel.setBounds (620, 270, 110, 16);
    outputGainKnob.setBounds (620, 290, 110, 110);
    outputGainValueLabel.setBounds (612, 402, 126, 18);

    signalSelectLabel.setBounds (470, 275, 80, 16);
    signalSelectSlider.setBounds (490, 295, 40, 90);
    signalSelectValueLabel.setBounds (474, 388, 72, 18);

    styleLabel.setBounds (806, 80, 180, 16);
    styleSlider.setBounds (806, 100, 180, 36);
    styleValueLabel.setBounds (806, 138, 180, 18);

    modeLabel.setBounds (806, 210, 180, 16);
    modeSlider.setBounds (806, 230, 180, 36);
    modeValueLabel.setBounds (806, 268, 180, 18);

    oversampleLabel.setBounds (806, 340, 180, 16);
    oversampleSlider.setBounds (806, 360, 180, 36);
    oversampleValueLabel.setBounds (806, 398, 180, 18);

    clipperBypassToggle.setBounds (40, 480, 150, 30);
    clipperStateLabel.setBounds (154, 482, 52, 24);
}
