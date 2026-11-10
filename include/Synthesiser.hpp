#pragma once

#include <JuceHeader.h>
#include <vector>
#include <cstdint>

#include "Voice.hpp"

/** Fixed-size polyphonic synthesiser.

    Owns its voices by value: setNumVoices() may allocate, so it must be called
    from the message thread (or before playback starts), never from processBlock().
*/
class CresizerSynthesiser
{
public:
    void setNumVoices (int count);
    void prepare (double newSampleRate);
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);

    /** allowTailOff == true lets the envelope release; false cuts the sound. */
    void stopAllNotes (bool allowTailOff = false);

    void setEnvelopeParameters (const Envelope::Parameters& newParameters);

    /** Realtime-safe waveform selection propagated to all voices; no allocation. */
    void setWaveform (WaveformType newWaveformType);

    /** Explicit limiter/clamp: guarantees output peak <= 1.0. */
    void applyPeakLimit (juce::AudioBuffer<float>& buffer);

    void startNote (int midiNoteNumber, float velocity);
    void stopNote (int midiNoteNumber, bool allowTailOff = true);

    Voice* findFreeVoice();
    Voice* findVoiceForNote (int midiNoteNumber);
    Voice* findOldestVoice();

private:
    void renderVoices (juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    std::vector<Voice> voices;
    double sampleRate = 44100.0;
    uint64_t noteCounter = 0;
};
