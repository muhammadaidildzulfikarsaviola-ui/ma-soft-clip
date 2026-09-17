#include "PluginProcessor.h"
#include "PluginEditor.h"

SoftClipAudioProcessorEditor::SoftClipAudioProcessorEditor (SoftClipAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel (&industrialLaf);

    // Setup Size Rasio Vintage Hardware (1024 x 576 px)
    setSize (1024, 576);

    // Helper Lambda untuk Inisialisasi Knob
    auto setupKnob = [this](juce::Slider& knob, juce::Label& label, const juce::String& text)
    {
        knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(knob);

        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::Font(12.0f, juce::Font::bold));
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
    };

    // Helper Lambda untuk Inisialisasi Horizontal Step Slider
    auto setupStepSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& text)
    {
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(slider);

        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::Font(13.0f, juce::Font::bold));
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
    };

    // 1. Controls Setup
    setupKnob(inputGainKnob, inputGainLabel, "GAIN");
    setupKnob(outputGainKnob, outputGainLabel, "OUTPUT");
    setupKnob(thresholdKnob, thresholdLabel, "THRESHOLD");
    setupKnob(kneeKnob, kneeLabel, "KNEE");

    setupStepSlider(styleSlider, styleLabel, "CLIPPER STYLE");
    setupStepSlider(modeSlider, modeLabel, "MODE");
    setupStepSlider(oversampleSlider, oversampleLabel, "OVERSAMPLING");

    // Signal Select Switch (IN / GR / OUT)
    signalSelectSlider.setSliderStyle(juce::Slider::LinearVertical);
    signalSelectSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(signalSelectSlider);
    signalSelectLabel.setText("SIGNAL", juce::dontSendNotification);
    signalSelectLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    signalSelectLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(signalSelectLabel);

    // Clipper Bypass Toggle Switch
    clipperBypassToggle.setButtonText("CLIPPER");
    addAndMakeVisible(clipperBypassToggle);

    // Meters
    addAndMakeVisible(inputMeterL);  addAndMakeVisible(inputMeterR);
    addAndMakeVisible(outputMeterL); addAndMakeVisible(outputMeterR);
    addAndMakeVisible(analogVuMeter);

    // Readout Digital Labels
    inputReadoutLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    inputReadoutLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(inputReadoutLabel);

    outputReadoutLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    outputReadoutLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(outputReadoutLabel);

    // 2. Hubungkan APVTS Attachments ke UI Controls
    auto& apvts = audioProcessor.getAPVTS();
    inputGainAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, IDs::inputGain, inputGainKnob));
    outputGainAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, IDs::outputGain, outputGainKnob));
    thresholdAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, IDs::threshold, thresholdKnob));
    kneeAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, IDs::knee, kneeKnob));

    styleAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, IDs::clipStyle, styleSlider));
    modeAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, IDs::mode, modeSlider));
    oversampleAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, IDs::oversample, oversampleSlider));
    signalSelectAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, IDs::signalSelect, signalSelectSlider));
    bypassAttach.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(apvts, IDs::bypass, clipperBypassToggle));

    // 3. Jalankan Realtime GUI Timer @ ~30 FPS (33 ms)
    startTimer(33);
}

SoftClipAudioProcessorEditor::~SoftClipAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void SoftClipAudioProcessorEditor::timerCallback()
{
    // Mengambil data real-time atomic dari Audio Thread tanpa blocking
    const float inPeakL = audioProcessor.getInputPeakL();
    const float inRmsL  = audioProcessor.getInputRmsL();
    const float outPeakL = audioProcessor.getOutputPeakL();
    const float outRmsL  = audioProcessor.getOutputRmsL();
    const float grDb     = audioProcessor.getCurrentGrDb();

    // Update Segmented LED Meters
    inputMeterL.setLevels (inPeakL, inRmsL);
    inputMeterR.setLevels (audioProcessor.getInputPeakR(), audioProcessor.getInputRmsR());

    outputMeterL.setLevels (outPeakL, outRmsL);
    outputMeterR.setLevels (audioProcessor.getOutputPeakR(), audioProcessor.getOutputRmsR());

    // Update Analog VU Meter GR (Ballistics Smoothing diolah internal di AnalogMeter)
    analogVuMeter.setGrValueDb (grDb);

    // Update Digital Readout
    const float inPeakDb = juce::Decibels::gainToDecibels(inPeakL, -60.0f);
    const float outPeakDb = juce::Decibels::gainToDecibels(outPeakL, -60.0f);
    inputReadoutLabel.setText (juce::String(inPeakDb, 1) + " dB\nLEVEL IN", juce::dontSendNotification);
    outputReadoutLabel.setText (juce::String(outPeakDb, 1) + " dB\nLEVEL OUT", juce::dontSendNotification);
}

void SoftClipAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Main Panel Metal Off-White / Brushed Cream
    g.fillAll (IndustrialStyle::metalBackground);

    // Draw Main Chassis Outline
    g.setColour (IndustrialStyle::panelBorder);
    g.drawRect (getLocalBounds().toFloat(), 2.0f);

    // Main Header Branding
    g.setColour (IndustrialStyle::textDark);
    g.setFont (juce::Font (26.0f, juce::Font::bold));
    g.drawText ("SOFT CLIP", 24, 16, 200, 32, juce::Justification::left);

    g.setFont (juce::Font (11.0f, juce::Font::bold));
    g.drawText ("CLEAN LOUDER TOGETHER", 170, 24, 200, 20, juce::Justification::left);

    g.setFont (juce::Font (20.0f, juce::Font::bold));
    g.drawText ("WADIDAW", getWidth() - 200, 14, 180, 24, juce::Justification::right);
    g.setFont (juce::Font (10.0f, juce::Font::bold));
    g.drawText ("AUDIO TOOLS", getWidth() - 200, 36, 180, 16, juce::Justification::right);

    // Panel Sections Backgrounds & Borders
    auto drawSectionBox = [&g](juce::Rectangle<float> bounds) {
        g.setColour (IndustrialStyle::metalPanel);
        g.fillRect (bounds);
        g.setColour (IndustrialStyle::panelBorder);
        g.drawRect (bounds, 1.5f);

        // Sekrup di 4 Sudut Panel
        IndustrialStyle::drawScrew (g, bounds.getX() + 8.0f, bounds.getY() + 8.0f, 9.0f, 0.5f);
        IndustrialStyle::drawScrew (g, bounds.getRight() - 8.0f, bounds.getY() + 8.0f, 9.0f, 1.2f);
        IndustrialStyle::drawScrew (g, bounds.getX() + 8.0f, bounds.getBottom() - 8.0f, 9.0f, 2.1f);
        IndustrialStyle::drawScrew (g, bounds.getRight() - 8.0f, bounds.getBottom() - 8.0f, 9.0f, 0.8f);
    };

    // Sub-panel Laying out
    drawSectionBox (juce::Rectangle<float> (16.0f, 60.0f, 210.0f, 380.0f));  // Metering Left
    drawSectionBox (juce::Rectangle<float> (236.0f, 60.0f, 540.0f, 480.0f)); // Center Main Controls
    drawSectionBox (juce::Rectangle<float> (786.0f, 60.0f, 222.0f, 480.0f)); // Right Switches Section
    drawSectionBox (juce::Rectangle<float> (16.0f, 450.0f, 210.0f, 90.0f));  // Bottom Left Clipper Toggle
    
    // Sekrup Sudut Sasis Utama
    IndustrialStyle::drawScrew (g, 12, 12, 12.0f, 0.3f);
    IndustrialStyle::drawScrew (g, getWidth() - 12, 12, 12.0f, 1.1f);
    IndustrialStyle::drawScrew (g, 12, getHeight() - 12, 12.0f, 2.4f);
    IndustrialStyle::drawScrew (g, getWidth() - 12, getHeight() - 12, 12.0f, 1.7f);

    // Footer Branding
    g.setFont (juce::Font (10.0f, juce::Font::bold));
    g.setColour (IndustrialStyle::textDark.withAlpha(0.7f));
    g.drawText ("WADIDAW AUDIO TOOLS", 24, getHeight() - 22, 200, 16, juce::Justification::left);
    g.drawText ("CLEAN LOUDER TOGETHER", getWidth() / 2 - 100, getHeight() - 22, 200, 16, juce::Justification::centred);
    g.drawText ("EST. 2025", getWidth() - 224, getHeight() - 22, 200, 16, juce::Justification::right);
}

void SoftClipAudioProcessorEditor::resized()
{
    // Layout Metering Kiri
    inputMeterL.setBounds (36, 90, 16, 260);
    inputMeterR.setBounds (56, 90, 16, 260);
    outputMeterL.setBounds (150, 90, 16, 260);
    outputMeterR.setBounds (170, 90, 16, 260);

    inputReadoutLabel.setBounds (30, 355, 60, 32);
    outputReadoutLabel.setBounds (144, 355, 60, 32);

    // Meter Analog VU Center Top
    analogVuMeter.setBounds (256, 80, 280, 170);

    // Knobs Center Section
    thresholdKnob.setBounds (560, 85, 90, 90);
    thresholdLabel.setBounds (560, 68, 90, 16);

    kneeKnob.setBounds (670, 85, 90, 90);
    kneeLabel.setBounds (670, 68, 90, 16);

    inputGainKnob.setBounds (300, 290, 110, 110);
    inputGainLabel.setBounds (300, 270, 110, 16);

    outputGainKnob.setBounds (620, 290, 110, 110);
    outputGainLabel.setBounds (620, 270, 110, 16);

    signalSelectSlider.setBounds (490, 295, 40, 90);
    signalSelectLabel.setBounds (470, 275, 80, 16);

    // Switches Kanan
    styleSlider.setBounds (806, 100, 180, 36);
    styleLabel.setBounds (806, 80, 180, 16);

    modeSlider.setBounds (806, 230, 180, 36);
    modeLabel.setBounds (806, 210, 180, 16);

    oversampleSlider.setBounds (806, 360, 180, 36);
    oversampleLabel.setBounds (806, 340, 180, 16);

    // Bottom Left Clipper Toggle
    clipperBypassToggle.setBounds (40, 480, 150, 30);
}