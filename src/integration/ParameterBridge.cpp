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

    jassert (attack != nullptr && decay != nullptr && sustain != nullptr
             && release != nullptr && waveform != nullptr && gain != nullptr);
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

    return p;
}
