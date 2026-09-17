#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace IDs
{
    static const juce::String inputGain   = "input_gain";
    static const juce::String outputGain  = "output_gain";
    static const juce::String threshold   = "threshold";
    static const juce::String knee        = "knee";
    static const juce::String clipStyle   = "clip_style";
    static const juce::String oversample  = "oversampling";
    static const juce::String mode        = "mode";
    static const juce::String bypass      = "bypass";
    static const juce::String signalSel   = "signal_select";
}

class ParameterHelpers
{
public:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

        // Input Gain: -12 dB to +12 dB
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { IDs::inputGain, 1 },
            "Input Gain",
            juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
            0.0f,
            juce::AudioParameterFloatAttributes().withLabel("dB")
        ));

        // Output Gain: -12 dB to +12 dB
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { IDs::outputGain, 1 },
            "Output Gain",
            juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
            0.0f,
            juce::AudioParameterFloatAttributes().withLabel("dB")
        ));

        // Threshold: -20 dB to 0 dB
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { IDs::threshold, 1 },
            "Threshold",
            juce::NormalisableRange<float>(-20.0f, 0.0f, 0.1f),
            0.0f,
            juce::AudioParameterFloatAttributes().withLabel("dB")
        ));

        // Knee: 0% to 100%
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { IDs::knee, 1 },
            "Knee",
            juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
            50.0f,
            juce::AudioParameterFloatAttributes().withLabel("%")
        ));

        // Clip Style: Soft (0), Medium (1), Hard (2)
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { IDs::clipStyle, 1 },
            "Clipper Style",
            juce::StringArray { "Soft", "Medium", "Hard" },
            0
        ));

        // Oversampling: 1x (0), 2x (1), 4x (2), 8x (3)
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { IDs::oversample, 1 },
            "Oversampling",
            juce::StringArray { "1x", "2x", "4x", "8x" },
            0
        ));

        // Mode: ST (0), MS (1), MB (2) -> ST default & primary
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { IDs::mode, 1 },
            "Mode",
            juce::StringArray { "ST", "MS", "MB" },
            0
        ));

        // Bypass / Clipper Switch: OFF (0), ON (1)
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID { IDs::bypass, 1 },
            "Clipper On/Off",
            true
        ));

        // Signal Select Switch: IN (0), GR (1), OUT (2)
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { IDs::signalSel, 1 },
            "Signal Monitor",
            juce::StringArray { "IN", "GR", "OUT" },
            1
        ));

        return { params.begin(), params.end() };
    }
};