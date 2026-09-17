#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/SoftClip.h"
#include "Utils/ParameterHelpers.h"
#include "Utils/AudioFifo.h"
#include "Utils/DebugLogger.h"
#include <atomic>

class SoftClipAudioProcessor  : public juce::AudioProcessor
{
public:
    SoftClipAudioProcessor();
    ~SoftClipAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Soft Clip"; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }

    // Atomic Readouts untuk GUI (Thread-safe, Non-blocking)
    float getInputRmsL() const noexcept { return inputRmsL.load(std::memory_order_relaxed); }
    float getInputRmsR() const noexcept { return inputRmsR.load(std::memory_order_relaxed); }
    float getInputPeakL() const noexcept { return inputPeakL.load(std::memory_order_relaxed); }
    float getInputPeakR() const noexcept { return inputPeakR.load(std::memory_order_relaxed); }

    float getOutputRmsL() const noexcept { return outputRmsL.load(std::memory_order_relaxed); }
    float getOutputRmsR() const noexcept { return outputRmsR.load(std::memory_order_relaxed); }
    float getOutputPeakL() const noexcept { return outputPeakL.load(std::memory_order_relaxed); }
    float getOutputPeakR() const noexcept { return outputPeakR.load(std::memory_order_relaxed); }

    float getCurrentGrDb() const noexcept { return currentGrDb.load(std::memory_order_relaxed); }
    float getPeakGrDb() const noexcept { return peakGrDb.load(std::memory_order_relaxed); }

    AudioFifo<float>& getWaveformFifo() noexcept { return waveformFifo; }

private:
    juce::AudioProcessorValueTreeState apvts;
    SoftClipDSP softClipDSP;

    // Cached raw parameter pointers (Terhubung penuh 100% ke APVTS)
    std::atomic<float>* inputGainParam    { nullptr };
    std::atomic<float>* outputGainParam   { nullptr };
    std::atomic<float>* thresholdParam    { nullptr };
    std::atomic<float>* kneeParam         { nullptr };
    std::atomic<float>* clipStyleParam    { nullptr };
    std::atomic<float>* oversampleParam   { nullptr };
    std::atomic<float>* modeParam         { nullptr };
    std::atomic<float>* bypassParam       { nullptr };
    std::atomic<float>* signalSelectParam { nullptr };

    // Atomic Monitoring Variables
    std::atomic<float> inputRmsL  { 0.0f }, inputRmsR  { 0.0f };
    std::atomic<float> inputPeakL { 0.0f }, inputPeakR { 0.0f };

    std::atomic<float> outputRmsL  { 0.0f }, outputRmsR  { 0.0f };
    std::atomic<float> outputPeakL { 0.0f }, outputPeakR { 0.0f };

    std::atomic<float> currentGrDb { 0.0f };
    std::atomic<float> peakGrDb    { 0.0f };

    // Lock-Free Waveform FIFO
    AudioFifo<float> waveformFifo { 8192 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoftClipAudioProcessor)
};