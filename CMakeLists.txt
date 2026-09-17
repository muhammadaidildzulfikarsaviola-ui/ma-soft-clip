cmake_minimum_required(VERSION 3.22)

project(SoftClip VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# JUCE
# ============================================================

set(JUCE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/JUCE"
    CACHE PATH
    "Path to JUCE"
)

if(NOT EXISTS "${JUCE_PATH}/CMakeLists.txt")
    message(FATAL_ERROR
        "JUCE not found at: ${JUCE_PATH}"
    )
endif()

add_subdirectory(
    "${JUCE_PATH}"
    "${CMAKE_BINARY_DIR}/JUCE"
)


# ============================================================
# FORMAT CONFIGURATION
# ============================================================

set(BUILD_VST2
    OFF
    CACHE BOOL
    "Build VST2 plugin"
)

set(VST2_SDK_PATH
    "D:/SDKs/VST2_SDK"
    CACHE PATH
    "Path to VST2 SDK"
)

set(PLUGIN_FORMATS
    VST3
    Standalone
)


# ============================================================
# OPTIONAL VST2
# ============================================================

if(BUILD_VST2)

    if(NOT EXISTS "${VST2_SDK_PATH}")
        message(FATAL_ERROR
            "VST2 SDK not found at: ${VST2_SDK_PATH}"
        )
    endif()

    juce_set_vst2_sdk_path("${VST2_SDK_PATH}")

    list(APPEND PLUGIN_FORMATS VST)

endif()


# ============================================================
# PLUGIN
# ============================================================

juce_add_plugin(SoftClip

    # --------------------------------------------------------
    # Manufacturer
    # --------------------------------------------------------

    COMPANY_NAME
        "Wadidaw Audio Tools"

    PLUGIN_MANUFACTURER_CODE
        Wdid


    # --------------------------------------------------------
    # Product
    # --------------------------------------------------------

    PRODUCT_NAME
        "Soft Clip"

    PLUGIN_NAME
        "Soft Clip"

    VERSION
        "1.0.0"

    BUNDLE_ID
        com.wadidaw.audiotools.softclip

    PLUGIN_CODE
        SfCl


    # --------------------------------------------------------
    # Plugin Type
    # --------------------------------------------------------

    IS_SYNTH
        FALSE

    NEEDS_MIDI_INPUT
        FALSE

    NEEDS_MIDI_OUTPUT
        FALSE

    IS_MIDI_EFFECT
        FALSE


    # --------------------------------------------------------
    # Formats
    # --------------------------------------------------------

    FORMATS
        ${PLUGIN_FORMATS}


    # --------------------------------------------------------
    # Build
    # --------------------------------------------------------

    COPY_PLUGIN_AFTER_BUILD
        TRUE


    # --------------------------------------------------------
    # VST3 Category
    # --------------------------------------------------------

    VST3_CATEGORIES
        "Fx Distortion"


    # --------------------------------------------------------
    # Description
    # --------------------------------------------------------

    DESCRIPTION
        "Soft clipping and harmonic processing."
)


# ============================================================
# SOURCES
# ============================================================

target_sources(SoftClip
    PRIVATE

        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp

        Source/DSP/SoftClip.cpp
        Source/DSP/Clipper.cpp

        Source/Meter/LevelMeter.cpp
        Source/Meter/AnalogMeter.cpp
)


# ============================================================
# INCLUDE DIRECTORIES
# ============================================================

target_include_directories(SoftClip
    PRIVATE

        Source
        Source/DSP
        Source/Meter
        Source/Utils
)


# ============================================================
# JUCE MODULES
# ============================================================

target_link_libraries(SoftClip
    PRIVATE

        juce::juce_audio_utils
        juce::juce_dsp

        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)