#pragma once

#include <JuceHeader.h>

#include "Envelope.hpp"
#include "Filter.hpp"
#include "Oscillator.hpp"

/** One polyphonic voice: a single oscillator through an ADSR envelope.

    Lifecycle: startNote() -> renderNextBlock()* -> isVoiceActive() == false.

    Render contract: renderNextBlock() ADDS its output to the buffer and never
    overwrites it, and must be realtime-safe (no allocation, no locks).
*/
class Voice
{
public:
    /** Called once before rendering; may allocate and resets the voice. */
    void prepare (double newSampleRate);

    void startNote (int midiNoteNumber, float newVelocity);
    /** Sounding voice at the same pitch: re-attack the envelope, keep phase and filter state. */
    void retriggerNote (int midiNoteNumber, float newVelocity);
    /** Sounding voice needed for another pitch: 2 ms fade-out, then start the new note. */
    void stealNote (int midiNoteNumber, float newVelocity);
    /** Drops a queued steal without disturbing the fading voice. */
    void cancelPendingNote() noexcept;
    void stopNote (float velocity, bool allowTailOff);
    void reset();

    bool isVoiceActive() const noexcept
    {
        return currentNote >= 0 || pendingNote >= 0 || fadeSamplesRemaining > 0;
    }
    int  getCurrentNote() const noexcept { return currentNote; }

    void setStartOrder (uint64_t order) noexcept { startOrder_ = order; }
    uint64_t getStartOrder() const noexcept { return startOrder_; }

    void setEnvelopeParameters (const Envelope::Parameters& newParameters);
    void setFilterParameters (const Filter::Parameters& newParameters);
    void setWaveform (WaveformType newWaveformType);

    // Extension points; no-ops until modulation is implemented.
    void handlePitchWheel (int /*newValue*/) {}
    void handleController (int /*controllerNumber*/, int /*newControllerValue*/) {}

    /** Adds this voice's output to the buffer. Must not be called when inactive. */
    void renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

private:
    Oscillator oscillator;
    Envelope envelope;
    Filter filter;
    double sampleRate = 44100.0;
    int currentNote = -1;
    float velocity = 0.0f;
    WaveformType currentWaveformType_ = WaveformType::sine;
    uint64_t startOrder_ = 0;

    // Declick state for stolen voices.
    int pendingNote = -1;
    float pendingVelocity = 0.0f;
    int fadeSamplesRemaining = 0;
    float fadeGain = 1.0f;
    float fadeStep = 0.0f;
};
