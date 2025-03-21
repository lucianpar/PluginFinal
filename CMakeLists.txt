cmake_minimum_required(VERSION 3.22)
project(AUDIO_PLUGIN_EXAMPLE VERSION 0.0.1)

add_subdirectory(JUCE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

juce_add_plugin(Wander-Prototype
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    PLUGIN_MANUFACTURER_CODE LCAN
    PLUGIN_CODE Wand
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Wander-V1"
    COPY_PLUGIN_AFTER_BUILD TRUE
)      # The name of the final executable, which can differ from the target name

target_sources(Wander-Prototype
    PRIVATE
        PluginEditor.cpp
        PluginProcessor.cpp
        Library.cpp)

juce_generate_juce_header(Wander-Prototype)

target_compile_definitions(Wander-Prototype
    PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(Wander-Prototype
    PRIVATE
        # AudioPluginData           # If we'd created a binary data target, we'd link to it here
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
