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

    // Enough for typical host MIDI plus GUI keyboard events; avoids audio-thread growth.
    mergedMidi_.ensureSize (4096);

    masterGain.reset (sampleRate, 0.02);
    masterGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (bridge.read().masterGainDb, -120.0f));
}

void CresizerAudioProcessor::releaseResources()
{
    keyboardState_.allNotesOff (0);
    synthesiser_.stopAllNotes();
}

void CresizerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Merge on-screen / QWERTY note events with MIDI arriving from the host.
    // mergedMidi_ keeps its capacity, so injecting GUI notes stays allocation-free.
    mergedMidi_.clear();
    mergedMidi_.addEvents (midiMessages, 0, -1, 0);
    keyboardState_.processNextMidiBuffer (mergedMidi_, 0, buffer.getNumSamples(), true);

    const auto params = bridge.read();

    synthesiser_.setEnvelopeParameters (params.envelope);
    synthesiser_.setFilterParameters (params.filter);
    synthesiser_.setWaveform (params.waveform);
    synthesiser_.processBlock (buffer, mergedMidi_);

    masterGain.setTargetValue (juce::Decibels::decibelsToGain (params.masterGainDb, -120.0f));
    masterGain.applyGain (buffer, buffer.getNumSamples());

    // Soft ceiling after gain; linear below 0.7, asymptotic to 0.95.
    synthesiser_.applySoftLimit (buffer);
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
