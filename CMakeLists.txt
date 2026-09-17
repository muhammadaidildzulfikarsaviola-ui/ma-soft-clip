cmake_minimum_required(VERSION 3.22)

project(SoftClip VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Menggunakan JUCE Lokal di D:\ma-soft-clip\JUCE
add_subdirectory(JUCE)

set(BUILD_VST2 OFF CACHE BOOL "Build VST2 plugin")

set(VST2_SDK_PATH
    "D:/SDKs/VST2_SDK"
    CACHE PATH
    "Path to VST2 SDK"
)

if(BUILD_VST2)
    if(NOT EXISTS "${VST2_SDK_PATH}")
        message(FATAL_ERROR "VST2 SDK not found at: ${VST2_SDK_PATH}")
    endif()
    juce_set_vst2_sdk_path("${VST2_SDK_PATH}")
endif()

juce_add_plugin(SoftClip
    COMPANY_NAME "Wadidaw Audio Tools"
    PLUGIN_MANUFACTURER_CODE Wdid
    PRODUCT_NAME "Soft Clip"
    PLUGIN_NAME "Soft Clip"
    VERSION "1.0.0"
    BUNDLE_ID com.wadidaw.audiotools.softclip
    PLUGIN_CODE SfCl
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    FORMATS
        VST3
        Standalone
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_CATEGORIES
        "Fx Distortion"
    DESCRIPTION
        "Soft clipping and harmonic processing."
)

if(BUILD_VST2)
    set_property(
        TARGET SoftClip
        PROPERTY JUCE_FORMATS
        "VST3;VST;Standalone"
    )
endif()

target_sources(SoftClip
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp

        Source/DSP/SoftClip.cpp
        Source/DSP/Clipper.cpp
        Source/DSP/Oversampling.cpp

        Source/Meter/LevelMeter.cpp
        Source/Meter/AnalogMeter.cpp
)

target_include_directories(SoftClip
    PRIVATE
        Source
        Source/DSP
        Source/Meter
        Source/Utils
)

target_link_libraries(SoftClip
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)