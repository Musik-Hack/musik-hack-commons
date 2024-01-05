project(LockFreeExample VERSION 0.0.1)

juce_add_plugin(LockFreeExample
    COMPANY_NAME "Musik Hack"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE MkHk
    PLUGIN_CODE LKFR
    FORMATS AU VST3 Standalone     
    PRODUCT_NAME "Lock-Free Queue Example"
    BUNDLE_ID com.musikhack.LockFreeExample)

target_sources(LockFreeExample
    PRIVATE
        Source/PluginEditor.cpp
        Source/PluginProcessor.cpp)


target_compile_definitions(LockFreeExample
    PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0
        MUSIKHACK_SAMPLES_DIR="${MUSIKHACK_SAMPLES_DIR}")


# juce_add_binary_data(AudioPluginData SOURCES ...)

target_link_libraries(LockFreeExample
    PRIVATE
        # AudioPluginData           # If we'd created a binary data target, we'd link to it here
        juce::juce_audio_utils
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_gui_basics
        juce::juce_dsp
        juce::juce_audio_formats
        musikhack::lockfree
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)