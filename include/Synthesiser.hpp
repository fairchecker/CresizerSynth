#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>

#include "SynthesiserVoice.hpp"

class SinewaveSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class Synthesiser
{
public:
    void addVoice(std::unique_ptr<SynthesiserVoice> voice);
    void prepare(double sampleRate);
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    void stopAllNotes();

private:
    void startNote(int midiNoteNumber, float velocity);
    void stopNote(int midiNoteNumber);
    void renderVoices(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);
    SynthesiserVoice* findFreeVoice();
    SynthesiserVoice* findVoiceForNote(int midiNoteNumber);

    std::vector<std::unique_ptr<SynthesiserVoice>> voices_;
    SinewaveSound sound_;
};
