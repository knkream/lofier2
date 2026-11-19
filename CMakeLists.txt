cmake_minimum_required(VERSION 3.15)
project(LoFiErV2 VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

if(DEFINED ENV{CI})
    set(SHOULD_COPY_PLUGIN FALSE)
else()
    set(SHOULD_COPY_PLUGIN TRUE)
endif()

juce_add_plugin(LoFiErV2
    COMPANY_NAME "KNK"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD ${SHOULD_COPY_PLUGIN}
    PLUGIN_MANUFACTURER_CODE Knkr
    PLUGIN_CODE Lfr2
    FORMATS VST3 Standalone
    PRODUCT_NAME "Lo-Fi-er v2"
)

target_sources(LoFiErV2 PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(LoFiErV2 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(LoFiErV2 PRIVATE
    juce::juce_audio_utils
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
