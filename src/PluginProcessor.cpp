#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"

CresizerAudioProcessor::CresizerAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    for (int i = 0; i < 8; ++i)
        synthesiser_.addVoice (std::make_unique<SynthesiserVoice>());
}

void CresizerAudioProcessor::prepareToPlay (double sampleRate, int)
{
    synthesiser_.prepare (sampleRate);
}

void CresizerAudioProcessor::releaseResources()
{
    synthesiser_.stopAllNotes();
}

void CresizerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    synthesiser_.processBlock (buffer, midiMessages);
}

juce::AudioProcessorEditor* CresizerAudioProcessor::createEditor()
{
    return new CresizerAudioProcessorEditor (*this);
}

bool CresizerAudioProcessor::hasEditor() const { return true; }
const juce::String CresizerAudioProcessor::getName() const { return JucePlugin_Name; }
bool CresizerAudioProcessor::acceptsMidi() const { return true; }
bool CresizerAudioProcessor::producesMidi() const { return false; }
double CresizerAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int CresizerAudioProcessor::getNumPrograms() { return 1; }
int CresizerAudioProcessor::getCurrentProgram() { return 0; }
void CresizerAudioProcessor::setCurrentProgram (int) {}
const juce::String CresizerAudioProcessor::getProgramName (int) { return {}; }
void CresizerAudioProcessor::changeProgramName (int, const juce::String&) {}
void CresizerAudioProcessor::getStateInformation (juce::MemoryBlock&) {}
void CresizerAudioProcessor::setStateInformation (const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CresizerAudioProcessor();
}
