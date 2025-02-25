cmake_minimum_required(VERSION 3.15)

project(plugin VERSION "0.0.1")

add_subdirectory(JUCE)

juce_add_console_app(convolution)

juce_generate_juce_header(convolution)

target_sources(convolution PRIVATE convolution.cpp)

target_compile_definitions(convolution PRIVATE
    JUCE_PLUGINHOST_LV2=1
    JUCE_PLUGINHOST_VST3=1
    JUCE_UNIT_TESTS=1
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    # This is a temporary workaround to allow builds to complete on Xcode 15.
    # Add -Wl,-ld_classic to the OTHER_LDFLAGS build setting if you need to
    # deploy to older versions of macOS.
    JUCE_SILENCE_XCODE_15_LINKER_WARNING=1)

target_link_libraries(convolution PRIVATE
    juce::juce_analytics
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_midi_ci
    juce::juce_opengl
    juce::juce_osc
    juce::juce_product_unlocking
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
