#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/IndustrialLookAndFeel.h"
#include "Meter/LevelMeter.h"
#include "Meter/AnalogMeter.h"


class SoftClipAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                      private juce::Timer
{
public:
    SoftClipAudioProcessorEditor (SoftClipAudioProcessor&);
    ~SoftClipAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    SoftClipAudioProcessor& audioProcessor;
    IndustrialLookAndFeel industrialLaf;

    // Meters Realtime
    Meter::LevelMeter inputMeterL, inputMeterR;
    Meter::LevelMeter outputMeterL, outputMeterR;
    Meter::AnalogMeter analogVuMeter;

    // Sliders / Knobs
    juce::Slider inputGainKnob;
    juce::Slider outputGainKnob;
    juce::Slider thresholdKnob;
    juce::Slider kneeKnob;

    // Step/Mode Switches (Clipper Style, Mode, Oversampling, Signal Select)
    juce::Slider styleSlider;
    juce::Slider modeSlider;
    juce::Slider oversampleSlider;
    juce::Slider signalSelectSlider;

    // Bypass Toggle
    juce::ToggleButton clipperBypassToggle;

    // Labels
    juce::Label inputGainLabel, outputGainLabel, thresholdLabel, kneeLabel;
    juce::Label styleLabel, modeLabel, oversampleLabel, signalSelectLabel;
    juce::Label inputReadoutLabel, outputReadoutLabel;

    // APVTS Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> kneeAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> styleAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modeAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> oversampleAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> signalSelectAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoftClipAudioProcessorEditor)
};