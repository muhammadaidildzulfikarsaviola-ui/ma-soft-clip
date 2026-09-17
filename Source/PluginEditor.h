#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"

class SoftClipAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    explicit SoftClipAudioProcessorEditor (SoftClipAudioProcessor&);
    ~SoftClipAudioProcessorEditor() override;

    void resized() override;

private:
    void timerCallback() override;
    void handleUiEvent (const juce::var& event);
    void sendStateToUi();

    SoftClipAudioProcessor& audioProcessor;
    juce::WebBrowserComponent webView;
    bool pageReady = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoftClipAudioProcessorEditor)
};
