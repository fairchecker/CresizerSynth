#pragma once

#include <JuceHeader.h>

#include "Envelope.hpp"
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
    void stopNote (float velocity, bool allowTailOff);
    void reset();

    bool isVoiceActive() const noexcept  { return currentNote >= 0; }
    int  getCurrentNote() const noexcept { return currentNote; }

    void setEnvelopeParameters (const Envelope::Parameters& newParameters);

    // Extension points; no-ops until modulation is implemented.
    void handlePitchWheel (int /*newValue*/) {}
    void handleController (int /*controllerNumber*/, int /*newControllerValue*/) {}

    /** Adds this voice's output to the buffer. Must not be called when inactive. */
    void renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

private:
    Oscillator oscillator;
    Envelope envelope;
    double sampleRate = 44100.0;
    int currentNote = -1;
    float velocity = 0.0f;
};
