#pragma once

#include <JuceHeader.h>
#include <vector>

#include "Voice.hpp"

/** Fixed-size polyphonic synthesiser.

    Owns its voices by value: setNumVoices() may allocate, so it must be called
    from the message thread (or before playback starts), never from processBlock().
*/
class Synthesiser
{
public:
    void setNumVoices (int count);
    void prepare (double newSampleRate);
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);

    /** allowTailOff == true lets the envelope release; false cuts the sound. */
    void stopAllNotes (bool allowTailOff = false);

    void setEnvelopeParameters (const Envelope::Parameters& newParameters);

private:
    void startNote (int midiNoteNumber, float velocity);
    void stopNote (int midiNoteNumber);
    void renderVoices (juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    Voice* findFreeVoice();
    Voice* findVoiceForNote (int midiNoteNumber);

    std::vector<Voice> voices;
    double sampleRate = 44100.0;
};
