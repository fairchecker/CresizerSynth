cmake_minimum_required(VERSION 3.22)

project(Cresizer
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_plugin(Cresizer
    VERSION 0.1.0
    COMPANY_NAME "Cresizer"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    FORMATS VST3 Standalone
    PRODUCT_NAME "Cresizer"
    VST3_CATEGORIES Instrument Synth
)

target_compile_definitions(Cresizer
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_sources(Cresizer PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
)

target_include_directories(Cresizer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Cresizer PRIVATE
    juce::juce_audio_utils
    juce::juce_gui_extra
)

juce_generate_juce_header(Cresizer)