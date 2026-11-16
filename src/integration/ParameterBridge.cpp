#include "integration/ParameterBridge.hpp"

#include "params/ParameterIDs.hpp"

ParameterBridge::ParameterBridge (juce::AudioProcessorValueTreeState& stateToUse)
    : state (stateToUse)
{
    attack   = state.getRawParameterValue (ParamID::attack);
    decay    = state.getRawParameterValue (ParamID::decay);
    sustain  = state.getRawParameterValue (ParamID::sustain);
    release  = state.getRawParameterValue (ParamID::release);
    waveform = state.getRawParameterValue (ParamID::waveform);
    gain     = state.getRawParameterValue (ParamID::gain);
    filterCutoff    = state.getRawParameterValue (ParamID::filterCutoff);
    filterResonance = state.getRawParameterValue (ParamID::filterResonance);
    filterType      = state.getRawParameterValue (ParamID::filterType);
    filterEnabled   = state.getRawParameterValue (ParamID::filterEnabled);

    jassert (attack != nullptr && decay != nullptr && sustain != nullptr
             && release != nullptr && waveform != nullptr && gain != nullptr
             && filterCutoff != nullptr && filterResonance != nullptr && filterType != nullptr
             && filterEnabled != nullptr);
}

SynthParameters ParameterBridge::read() const
{
    SynthParameters p;

    p.envelope.attack  = attack->load (std::memory_order_relaxed);
    p.envelope.decay   = decay->load (std::memory_order_relaxed);
    p.envelope.sustain = sustain->load (std::memory_order_relaxed);
    p.envelope.release = release->load (std::memory_order_relaxed);

    p.waveform = static_cast<WaveformType> (juce::roundToInt (waveform->load (std::memory_order_relaxed)));
    p.masterGainDb = gain->load (std::memory_order_relaxed);

    p.filter.cutoffHz = filterCutoff->load (std::memory_order_relaxed);
    p.filter.resonance = filterResonance->load (std::memory_order_relaxed);
    p.filter.type = static_cast<FilterType> (juce::roundToInt (filterType->load (std::memory_order_relaxed)));
    p.filter.enabled = filterEnabled->load (std::memory_order_relaxed) > 0.5f;

    return p;
}
