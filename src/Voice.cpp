#include "Voice.hpp"

namespace
{
    // Static per-voice headroom (1/sqrt(8)) so an 8-voice chord has room before
    // the output soft ceiling. Research: fixed trim beats dynamic 1/sqrt(N).
    constexpr float kVoiceHeadroom = 0.35355339f;
    constexpr float kStealFadeSeconds = 0.002f;   // 2 ms declick ramp
}

void Voice::prepare (double newSampleRate)
{
    sampleRate = newSampleRate;

    oscillator.prepare (sampleRate);
    envelope.prepare (sampleRate);
    filter.prepare (sampleRate);
    reset();
}

void Voice::startNote (int midiNoteNumber, float newVelocity)
{
    pendingNote = -1;
    pendingVelocity = 0.0f;
    fadeSamplesRemaining = 0;
    fadeGain = 1.0f;
    fadeStep = 0.0f;

    currentNote = midiNoteNumber;
    velocity = newVelocity;

    oscillator.setFrequency (juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber));
    oscillator.reset();
    filter.reset();

    envelope.reset();
    envelope.noteOn();
}

void Voice::retriggerNote (int midiNoteNumber, float newVelocity)
{
    // Cancelling a steal in progress keeps the old signal continuous.
    pendingNote = -1;
    pendingVelocity = 0.0f;
    fadeSamplesRemaining = 0;
    fadeGain = 1.0f;
    fadeStep = 0.0f;

    currentNote = midiNoteNumber;
    velocity = newVelocity;

    oscillator.setFrequency (juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber));
    // juce::ADSR::noteOn() re-enters attack from the current level: no jump.
    envelope.noteOn();
}

void Voice::stealNote (int midiNoteNumber, float newVelocity)
{
    if (!isVoiceActive())
    {
        startNote (midiNoteNumber, newVelocity);
        return;
    }

    pendingNote = midiNoteNumber;
    pendingVelocity = newVelocity;

    if (fadeSamplesRemaining == 0)
    {
        fadeSamplesRemaining = juce::jmax (1, static_cast<int> (kStealFadeSeconds * sampleRate));
        fadeGain = 1.0f;
        fadeStep = 1.0f / static_cast<float> (fadeSamplesRemaining);
    }
}

void Voice::cancelPendingNote() noexcept
{
    pendingNote = -1;
    pendingVelocity = 0.0f;
}

void Voice::stopNote (float, bool allowTailOff)
{
    if (allowTailOff)
    {
        envelope.noteOff();
    }
    else
    {
        envelope.reset();
        currentNote = -1;
    }
}

void Voice::reset()
{
    envelope.reset();
    oscillator.reset();
    filter.reset();

    currentNote = -1;
    velocity = 0.0f;

    pendingNote = -1;
    pendingVelocity = 0.0f;
    fadeSamplesRemaining = 0;
    fadeGain = 1.0f;
    fadeStep = 0.0f;
}

void Voice::setWaveform (WaveformType newWaveformType)
{
    if (currentWaveformType_ != newWaveformType)
    {
        currentWaveformType_ = newWaveformType;
        oscillator.setWaveform (newWaveformType);
    }
}

void Voice::setEnvelopeParameters (const Envelope::Parameters& newParameters)
{
    envelope.setParameters (newParameters);
}

void Voice::setFilterParameters (const Filter::Parameters& newParameters)
{
    filter.setParameters (newParameters);
}

void Voice::renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    if (!isVoiceActive())
        return;

    const auto numChannels = buffer.getNumChannels();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto env = envelope.getNextSample();
        const auto filtered = filter.processSample (oscillator.getNextSample());
        auto value = filtered * env * velocity * kVoiceHeadroom;

        if (fadeSamplesRemaining > 0)
        {
            value *= fadeGain;
            fadeGain = juce::jmax (0.0f, fadeGain - fadeStep);
            --fadeSamplesRemaining;

            if (fadeSamplesRemaining == 0 && pendingNote >= 0)
            {
                const int note = pendingNote;
                const float pendingVel = pendingVelocity;
                reset();
                startNote (note, pendingVel);
            }
        }

        for (int channel = 0; channel < numChannels; ++channel)
            buffer.addSample (channel, startSample + sample, value);
    }

    if (!envelope.isActive() && pendingNote < 0 && fadeSamplesRemaining == 0)
        currentNote = -1;
}
