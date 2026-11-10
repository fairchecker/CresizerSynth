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
    src/Oscillator.cpp
    src/Sinewave.cpp
    src/Waveform.cpp
    src/Envelope.cpp
    src/Voice.cpp
    src/Synthesiser.cpp
    src/params/ParameterLayout.cpp
    src/integration/ParameterBridge.cpp
)

target_include_directories(Cresizer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Cresizer PRIVATE
    juce::juce_audio_utils
    juce::juce_gui_extra
)

juce_generate_juce_header(Cresizer)

include(CTest)
if(BUILD_TESTING)
    add_executable(CresizerTests tests/dsp_tests.cpp)
    target_include_directories(CresizerTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
        ${CMAKE_BINARY_DIR}/Cresizer_artefacts/JuceLibraryCode
    )
    target_link_libraries(CresizerTests PRIVATE Cresizer juce::juce_audio_utils)
    add_test(NAME CresizerTests COMMAND CresizerTests)
endif()
