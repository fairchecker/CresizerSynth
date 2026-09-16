#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"

CresizerAudioProcessor::CresizerAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

void CresizerAudioProcessor::prepareToPlay (double, int) {}
void CresizerAudioProcessor::releaseResources() {}
void CresizerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    // Пропускаем звук как есть
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());
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