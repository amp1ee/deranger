juce_add_plugin(Deranger

    COMPANY_NAME "Amplee"                    # Specify the name of the plugin's author
    COMPANY_COPYRIGHT "Amplee"
    COMPANY_WEBSITE "https://github.com/amp1ee/"
    COMPANY_EMAIL "hid.den@gmail.com"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE

    PLUGIN_MANUFACTURER_CODE Juce
    PLUGIN_CODE EfRa
    FORMATS 
        VST3
        Standalone
    VST3_CATEGORIES "Fx"
    AU_MAIN_TYPE "kAudioUnitType_Effect"

    PRODUCT_NAME "Deranger"
)

target_compile_features(Deranger PUBLIC cxx_std_17)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(Deranger PRIVATE -finput-charset=UTF-8)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(Deranger PRIVATE /utf-8)
endif()

target_compile_definitions(Deranger
    PUBLIC
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    JUCE_VST3_CAN_REPLACE_VST2=0
    # JUCE_DISPLAY_SPLASH_SCREEN=0 #if your plugin is distributed with GPL license or paid
)

juce_add_binary_data(BinaryAssets SOURCES
    ${CMAKE_SOURCE_DIR}/assets/juce-logo.svg
    ${CMAKE_SOURCE_DIR}/assets/amplee.svg
)

target_sources(Deranger PRIVATE
    PluginProcessor.cpp
    PluginProcessor.h
    PluginEditor.cpp
    PluginEditor.h
    core/RoutingNode.cpp
    core/RoutingNode.h
    core/RackProcessor.h
    effects/RackEffect.h
    effects/ReverbProcessor.h
    effects/DelayProcessor.h
    effects/FlangerProcessor.h
    ui/LabelWithBackground.h
    ui/ToggleLookAndFeel.h
    ui/VisualizerComponent.h
    ui/DynamicLookAndFeel.h
)

target_link_libraries(Deranger PRIVATE
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

juce_generate_juce_header(Deranger)