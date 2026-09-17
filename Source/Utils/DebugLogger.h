#pragma once

#include <juce_core/juce_core.h>
#include <atomic>
#include <iostream>

struct DebugInfo
{
    double sampleRate { 44100.0 };
    int bufferSize { 512 };
    int numChannels { 2 };

    float inputRmsDb { -100.0f };
    float inputPeakDb { -100.0f };

    float thresholdDb { 0.0f };
    float kneePercent { 50.0f };
    int clipStyleIndex { 0 };
    int oversampleIndex { 0 };

    float currentGrDb { 0.0f };
    float peakGrDb { 0.0f };

    float outputRmsDb { -100.0f };
    float outputPeakDb { -100.0f };
};

class DebugLogger
{
public:
    DebugLogger() = default;

    static void printDebugOutput(const DebugInfo& info)
    {
        static const char* styles[] = { "Soft", "Medium", "Hard" };
        static const char* oversamples[] = { "1x", "2x", "4x", "8x" };

        std::cout << "\n========================================\n"
                  << "           SOFT CLIP DEBUG              \n"
                  << "========================================\n"
                  << "AUDIO\n"
                  << "Sample Rate : " << info.sampleRate << " Hz\n"
                  << "Buffer      : " << info.bufferSize << "\n"
                  << "Channels    : " << info.numChannels << "\n\n"
                  << "INPUT\n"
                  << "RMS         : " << juce::String(info.inputRmsDb, 1) << " dBFS\n"
                  << "Peak        : " << juce::String(info.inputPeakDb, 1) << " dBFS\n\n"
                  << "PROCESSING\n"
                  << "Threshold   : " << juce::String(info.thresholdDb, 1) << " dB\n"
                  << "Knee        : " << juce::String(info.kneePercent, 0) << " %\n"
                  << "Clip Style  : " << styles[juce::jlimit(0, 2, info.clipStyleIndex)] << "\n"
                  << "Oversample  : " << oversamples[juce::jlimit(0, 3, info.oversampleIndex)] << "\n\n"
                  << "GAIN REDUCTION\n"
                  << "Current     : " << juce::String(info.currentGrDb, 1) << " dB\n"
                  << "Peak        : " << juce::String(info.peakGrDb, 1) << " dB\n\n"
                  << "OUTPUT\n"
                  << "RMS         : " << juce::String(info.outputRmsDb, 1) << " dBFS\n"
                  << "Peak        : " << juce::String(info.outputPeakDb, 1) << " dBFS\n"
                  << "========================================\n" << std::endl;
    }
};