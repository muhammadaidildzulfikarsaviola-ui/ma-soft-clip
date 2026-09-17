#include "PluginProcessor.h"
#include "PluginEditor.h"

SoftClipAudioProcessor::SoftClipAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       apvts (*this, nullptr, "Parameters", ParameterHelpers::createParameterLayout())
#endif
{
    // Caching 9 dari 9 parameter APVTS secara lengkap
    inputGainParam    = apvts.getRawParameterValue (IDs::inputGain);
    outputGainParam   = apvts.getRawParameterValue (IDs::outputGain);
    thresholdParam    = apvts.getRawParameterValue (IDs::threshold);
    kneeParam         = apvts.getRawParameterValue (IDs::knee);
    clipStyleParam    = apvts.getRawParameterValue (IDs::clipStyle);
    oversampleParam   = apvts.getRawParameterValue (IDs::oversample);
    modeParam         = apvts.getRawParameterValue (IDs::mode);
    bypassParam       = apvts.getRawParameterValue (IDs::bypass);
    signalSelectParam = apvts.getRawParameterValue (IDs::signalSelect);
}

SoftClipAudioProcessor::~SoftClipAudioProcessor() {}

void SoftClipAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const int numChannels = getTotalNumInputChannels();
    softClipDSP.prepare (sampleRate, samplesPerBlock, std::max (1, numChannels));

    waveformFifo.setSize (8192);

    inputRmsL.store (0.0f);  inputRmsR.store (0.0f);
    inputPeakL.store (0.0f); inputPeakR.store (0.0f);
    outputRmsL.store (0.0f); outputRmsR.store (0.0f);
    outputPeakL.store (0.0f); outputPeakR.store (0.0f);
    currentGrDb.store (0.0f);
    peakGrDb.store (0.0f);
}

void SoftClipAudioProcessor::releaseResources()
{
    softClipDSP.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SoftClipAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}
#endif

void SoftClipAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);

    if (numSamples == 0)
        return;

    // 1. Update DSP Parameters
    softClipDSP.setInputGainDb (inputGainParam->load());
    softClipDSP.setOutputGainDb (outputGainParam->load());
    softClipDSP.setThresholdDb (thresholdParam->load());
    softClipDSP.setKneePercent (kneeParam->load());
    softClipDSP.setClipStyle (static_cast<ClipStyle>(static_cast<int>(clipStyleParam->load())));
    softClipDSP.setOversamplingIndex (static_cast<int>(oversampleParam->load()));
    softClipDSP.setMode (static_cast<ProcessingMode>(static_cast<int>(modeParam->load())));
    softClipDSP.setSignalSelect (static_cast<SignalSelect>(static_cast<int>(signalSelectParam->load())));
    softClipDSP.setBypass (bypassParam->load() > 0.5f);
    softClipDSP.setMode(static_cast<ProcessingMode>(static_cast<int>(modeParam->load())));
    softClipDSP.setSignalSelect(static_cast<SignalSelect>(static_cast<int>(signalSelectParam->load())));

    setLatencySamples (juce::roundToInt (softClipDSP.getLatencyInSamples()));

    // 2. Input Metering (Dynamic channel check - Mono & Stereo Safe)
    if (totalNumInputChannels > 0)
    {
        inputPeakL.store (buffer.getMagnitude (0, 0, numSamples), std::memory_order_relaxed);
        inputRmsL.store (buffer.getRMSLevel (0, 0, numSamples), std::memory_order_relaxed);
    }
    if (totalNumInputChannels > 1)
    {
        inputPeakR.store (buffer.getMagnitude (1, 0, numSamples), std::memory_order_relaxed);
        inputRmsR.store (buffer.getRMSLevel (1, 0, numSamples), std::memory_order_relaxed);
    }
    else
    {
        inputPeakR.store (inputPeakL.load(), std::memory_order_relaxed);
        inputRmsR.store (inputRmsL.load(), std::memory_order_relaxed);
    }

    // 3. Core Processing
    float blockMaxGrDb = 0.0f;
    softClipDSP.process (buffer, blockMaxGrDb);

    // 4. Output Metering
    if (totalNumOutputChannels > 0)
    {
        outputPeakL.store (buffer.getMagnitude (0, 0, numSamples), std::memory_order_relaxed);
        outputRmsL.store (buffer.getRMSLevel (0, 0, numSamples), std::memory_order_relaxed);
    }
    if (totalNumOutputChannels > 1)
    {
        outputPeakR.store (buffer.getMagnitude (1, 0, numSamples), std::memory_order_relaxed);
        outputRmsR.store (buffer.getRMSLevel (1, 0, numSamples), std::memory_order_relaxed);
    }
    else
    {
        outputPeakR.store (outputPeakL.load(), std::memory_order_relaxed);
        outputRmsR.store (outputRmsL.load(), std::memory_order_relaxed);
    }

    // 5. Pure DSP Gain Reduction
    currentGrDb.store (blockMaxGrDb, std::memory_order_relaxed);
    
    float prevPeakGr = peakGrDb.load (std::memory_order_relaxed);
    if (blockMaxGrDb >= prevPeakGr)
    {
        peakGrDb.store (blockMaxGrDb, std::memory_order_relaxed);
    }
    else
    {
        peakGrDb.store (std::max (0.0f, prevPeakGr - 0.05f), std::memory_order_relaxed);
    }

    // 6. Push Output Samples to Lock-Free FIFO
    if (totalNumOutputChannels > 0)
    {
        const float* channelData = buffer.getReadPointer (0);
        waveformFifo.write (channelData, numSamples);
    }
}

juce::AudioProcessorEditor* SoftClipAudioProcessor::createEditor()
{
    return new SoftClipAudioProcessorEditor (*this);
}

void SoftClipAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void SoftClipAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SoftClipAudioProcessor();
}