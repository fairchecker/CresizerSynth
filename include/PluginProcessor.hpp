#pragma once

#include <JuceHeader.h>

#include "Synthesiser.hpp"
#include "params/ParameterLayout.hpp"
#include "integration/ParameterBridge.hpp"

class CresizerAudioProcessor : public juce::AudioProcessor
{
public:
    CresizerAudioProcessor();
    ~CresizerAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    /** GUI uses attachments / host-notifying edits, not DSP setters. */
    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }
    const juce::AudioProcessorValueTreeState& getAPVTS() const noexcept { return apvts; }

private:
    juce::AudioProcessorValueTreeState apvts;   // ОБЯЗАТЕЛЬНО объявлен раньше bridge
    ParameterBridge bridge;
    CresizerSynthesiser synthesiser_;
    juce::SmoothedValue<float> masterGain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CresizerAudioProcessor)
};
