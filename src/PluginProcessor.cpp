#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"

#include "params/ParameterIDs.hpp"

CresizerAudioProcessor::CresizerAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout()),
      bridge (apvts)
{
    synthesiser_.setNumVoices (8);
}

void CresizerAudioProcessor::prepareToPlay (double sampleRate, int)
{
    synthesiser_.prepare (sampleRate);

    masterGain.reset (sampleRate, 0.02);
    masterGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (bridge.read().masterGainDb, -120.0f));
}

void CresizerAudioProcessor::releaseResources()
{
    synthesiser_.stopAllNotes();
}

void CresizerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const auto params = bridge.read();

    synthesiser_.setEnvelopeParameters (params.envelope);
    synthesiser_.setWaveform (params.waveform);
    synthesiser_.processBlock (buffer, midiMessages);

    masterGain.setTargetValue (juce::Decibels::decibelsToGain (params.masterGainDb, -120.0f));
    masterGain.applyGain (buffer, buffer.getNumSamples());

    // Explicit limiter/clamp after gain: guarantee peak <= 1.0
    synthesiser_.applyPeakLimit (buffer);
}

juce::AudioProcessorEditor* CresizerAudioProcessor::createEditor()
{
    return new CresizerAudioProcessorEditor (*this);
}

bool CresizerAudioProcessor::hasEditor() const { return true; }
const juce::String CresizerAudioProcessor::getName() const { return JucePlugin_Name; }
bool CresizerAudioProcessor::acceptsMidi() const { return true; }
bool CresizerAudioProcessor::producesMidi() const { return false; }
double CresizerAudioProcessor::getTailLengthSeconds() const
{
    // Maximum release supported by the parameter contract.
    return 20.0;
}
int CresizerAudioProcessor::getNumPrograms() { return 1; }
int CresizerAudioProcessor::getCurrentProgram() { return 0; }
void CresizerAudioProcessor::setCurrentProgram (int) {}
const juce::String CresizerAudioProcessor::getProgramName (int) { return {}; }
void CresizerAudioProcessor::changeProgramName (int, const juce::String&) {}
void CresizerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void CresizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (data == nullptr || sizeInBytes <= 0)
        return;

    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        const auto state = juce::ValueTree::fromXml (*xml);
        if (state.isValid() && state.hasType (apvts.state.getType()))
            apvts.replaceState (state);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CresizerAudioProcessor();
}
