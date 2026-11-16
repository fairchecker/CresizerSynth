#include "params/ParameterLayout.hpp"

#include "params/ParameterIDs.hpp"

static std::unique_ptr<juce::RangedAudioParameter> makeFloat (const char* id,
                                                              const char* name,
                                                              juce::NormalisableRange<float> range,
                                                              float defaultValue,
                                                              juce::AudioParameterFloatAttributes attributes = {})
{
    return std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { id, 1 }, name, range, defaultValue, attributes);
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (makeFloat (ParamID::attack,  "Attack",  { 0.001f, 10.0f, 0.0f, 0.3f }, 0.01f));
    layout.add (makeFloat (ParamID::decay,   "Decay",   { 0.001f, 10.0f, 0.0f, 0.3f }, 0.10f));
    layout.add (makeFloat (ParamID::sustain, "Sustain", { 0.0f,   1.0f,  0.001f },      0.80f));
    layout.add (makeFloat (ParamID::release, "Release", { 0.001f, 20.0f, 0.0f, 0.3f }, 0.20f));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamID::waveform, 1 },
        "Waveform",
        juce::StringArray { "Sine", "Saw", "Square", "Triangle" },
        0));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamID::gain, 1 },
        "Master Gain",
        juce::NormalisableRange<float> { -120.0f, 0.0f, 0.1f },
        -6.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamID::filterEnabled, 1 },
        "Filter Enabled",
        false));

    layout.add (makeFloat (ParamID::filterCutoff, "Filter Cutoff",
                           juce::NormalisableRange<float> { 20.0f, 20000.0f, 0.1f, 0.25f },
                           20000.0f,
                           juce::AudioParameterFloatAttributes()
                               .withStringFromValueFunction ([] (float value, int)
                                   { return juce::String (juce::roundToInt (value)) + " Hz"; })));
    layout.add (makeFloat (ParamID::filterResonance, "Filter Resonance",
                           juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f },
                           0.0f));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamID::filterType, 1 },
        "Filter Type",
        juce::StringArray { "Lowpass", "Highpass" },
        0));

    return layout;
}
