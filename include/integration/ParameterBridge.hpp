#pragma once

#include <JuceHeader.h>

#include "params/ParameterSnapshot.hpp"

class ParameterBridge
{
public:
    explicit ParameterBridge (juce::AudioProcessorValueTreeState& stateToUse);

    /** Realtime-safe: only atomic loads, no allocation. */
    SynthParameters read() const;

private:
    juce::AudioProcessorValueTreeState& state;

    std::atomic<float>* attack = nullptr;
    std::atomic<float>* decay = nullptr;
    std::atomic<float>* sustain = nullptr;
    std::atomic<float>* release = nullptr;
    std::atomic<float>* waveform = nullptr;
    std::atomic<float>* gain = nullptr;
    std::atomic<float>* filterCutoff = nullptr;
    std::atomic<float>* filterResonance = nullptr;
    std::atomic<float>* filterType = nullptr;
    std::atomic<float>* filterEnabled = nullptr;
};
