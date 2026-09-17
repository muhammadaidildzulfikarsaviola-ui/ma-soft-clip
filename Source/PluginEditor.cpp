#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr const char* devUiUrl = "http://localhost:5173/";

    juce::WebBrowserComponent::Options makeWebViewOptions()
    {
        auto options = juce::WebBrowserComponent::Options()
            .withBackend (juce::WebBrowserComponent::Options::Backend::webview2)
            .withNativeIntegrationEnabled (true)
            .withWinWebView2Options (
                juce::WebBrowserComponent::Options::WinWebView2()
                    .withUserDataFolder (
                        juce::File::getSpecialLocation (
                            juce::File::tempDirectory
                        ).getChildFile ("SoftClipWebView2")
                    )
            );

        DBG ("Soft Clip: WebView2 options supported = "
             << juce::WebBrowserComponent::areOptionsSupported (options));

        return options;
    }

    juce::var makeUiState (SoftClipAudioProcessor& processor)
    {
        auto& apvts = processor.getAPVTS();
        auto state = juce::DynamicObject::Ptr (new juce::DynamicObject());

        const auto read = [&apvts] (const juce::String& id) -> float
        {
            if (auto* value = apvts.getRawParameterValue (id))
                return value->load (std::memory_order_relaxed);
            return 0.0f;
        };

        state->setProperty ("gain", read (IDs::inputGain));
        state->setProperty ("output", read (IDs::outputGain));
        state->setProperty ("threshold", read (IDs::threshold));
        state->setProperty ("knee", read (IDs::knee));
        state->setProperty ("style", juce::roundToInt (read (IDs::clipStyle)));
        state->setProperty ("mode", juce::roundToInt (read (IDs::mode)));
        state->setProperty ("oversampling", juce::roundToInt (read (IDs::oversample)));
        state->setProperty ("signal", juce::roundToInt (read (IDs::signalSelect)));
        state->setProperty ("clipper", read (IDs::bypass) > 0.5f);

        const auto inputPeak = juce::jmax (processor.getInputPeakL(), processor.getInputPeakR());
        const auto outputPeak = juce::jmax (processor.getOutputPeakL(), processor.getOutputPeakR());
        state->setProperty ("inputDb", juce::Decibels::gainToDecibels (inputPeak, -60.0f));
        state->setProperty ("outputDb", juce::Decibels::gainToDecibels (outputPeak, -60.0f));
        state->setProperty ("grDb", -juce::jmax (0.0f, processor.getCurrentGrDb()));

        return juce::var (state.get());
    }
}

SoftClipAudioProcessorEditor::SoftClipAudioProcessorEditor (SoftClipAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p),
      webView (makeWebViewOptions()
          .withEventListener ("uiReady", [this] (const juce::var&)
          {
              pageReady = true;
              DBG ("Soft Clip: UI sent uiReady");
              sendStateToUi();
          })
          .withEventListener ("setParameter", [this] (const juce::var& event)
          {
              handleUiEvent (event);
          }))
{
    setSize (1024, 576);
    addAndMakeVisible (webView);

    if (! juce::WebBrowserComponent::areOptionsSupported (makeWebViewOptions()))
    {
        juce::AlertWindow::showMessageBoxAsync (
            juce::MessageBoxIconType::WarningIcon,
            "Soft Clip - WebView2",
            "JUCE could not use the WebView2 backend.\n\n"
            "The browser may fall back to Internet Explorer.\n"
            "Check the WebView2 runtime / loader installation.",
            "OK");
    }

#if SOFTCLIP_UI_DEV_SERVER
    webView.goToURL (devUiUrl);
#else
    // Release mode will use an embedded resource provider once the UI is frozen.
    webView.goToURL (devUiUrl);
#endif

    startTimer (33);
}

SoftClipAudioProcessorEditor::~SoftClipAudioProcessorEditor()
{
    stopTimer();
}

void SoftClipAudioProcessorEditor::resized()
{
    webView.setBounds (getLocalBounds());
}

void SoftClipAudioProcessorEditor::handleUiEvent (const juce::var& event)
{
    if (! event.isObject())
        return;

    auto* object = event.getDynamicObject();
    if (object == nullptr)
        return;

    const auto parameterId = object->getProperty ("id").toString();
    if (parameterId.isEmpty())
        return;

    if (auto* parameter = audioProcessor.getAPVTS().getParameter (parameterId))
    {
        const auto requestedValue = static_cast<float> (object->getProperty ("value"));

        if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (parameter))
        {
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost (ranged->convertTo0to1 (requestedValue));
            parameter->endChangeGesture();
        }
    }
}

void SoftClipAudioProcessorEditor::sendStateToUi()
{
    if (! pageReady)
        return;

    const auto json = juce::JSON::toString (makeUiState (audioProcessor));
    webView.evaluateJavascript ("window.setPluginState && window.setPluginState(" + json + ");");
}

void SoftClipAudioProcessorEditor::timerCallback()
{
    sendStateToUi();
}
