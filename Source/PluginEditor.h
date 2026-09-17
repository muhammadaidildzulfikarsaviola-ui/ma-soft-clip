#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/IndustrialLookAndFeel.h"
#include "Meter/LevelMeter.h"
#include "Meter/AnalogMeter.h"

class SoftClipAudioProcessorEditor : public juce::AudioProcessorEditor,
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

    // Realtime meters
    Meter::LevelMeter inputMeterL, inputMeterR;
    Meter::LevelMeter outputMeterL, outputMeterR;
    Meter::AnalogMeter analogVuMeter;

    // Main controls
    juce::Slider inputGainKnob;
    juce::Slider outputGainKnob;
    juce::Slider thresholdKnob;
    juce::Slider kneeKnob;

    // Step / mode controls
    juce::Slider styleSlider;
    juce::Slider modeSlider;
    juce::Slider oversampleSlider;
    juce::Slider signalSelectSlider;

    juce::ToggleButton clipperBypassToggle;

    // Parameter name labels
    juce::Label inputGainLabel, outputGainLabel, thresholdLabel, kneeLabel;
    juce::Label styleLabel, modeLabel, oversampleLabel, signalSelectLabel;

    // Live parameter value labels
    juce::Label inputGainValueLabel, outputGainValueLabel;
    juce::Label thresholdValueLabel, kneeValueLabel;
    juce::Label styleValueLabel, modeValueLabel, oversampleValueLabel;
    juce::Label signalSelectValueLabel, clipperStateLabel;

    // Live meter readouts
    juce::Label inputReadoutLabel, outputReadoutLabel;

    // APVTS attachments
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
