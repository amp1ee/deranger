juce_add_plugin(ExamplePlugin
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE

    PLUGIN_MANUFACTURER_CODE Juce
    PLUGIN_CODE EfRa
    FORMATS VST3 AU Standalone

    VST3_CATEGORIES "Fx"
    AU_MAIN_TYPE "kAudioUnitType_Effect"

    PRODUCT_NAME "ExamplePlugin"
)

target_compile_features(ExamplePlugin PUBLIC cxx_std_17)

juce_add_binary_data(BinaryAssets SOURCES
    ${CMAKE_SOURCE_DIR}/assets/juce-logo.svg
)

target_sources(ExamplePlugin PRIVATE
    PluginProcessor.cpp
    PluginProcessor.h
    PluginEditor.cpp
    PluginEditor.h
    RackProcessor.h
    ReverbProcessor.h
)

target_link_libraries(ExamplePlugin PRIVATE
    BinaryAssets

    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)

juce_generate_juce_header(ExamplePlugin)