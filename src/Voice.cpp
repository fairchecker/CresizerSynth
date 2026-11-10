#include "Voice.hpp"

void Voice::prepare (double newSampleRate)
{
    sampleRate = newSampleRate;

    oscillator.prepare (sampleRate);
    envelope.prepare (sampleRate);
    reset();
}

void Voice::startNote (int midiNoteNumber, float newVelocity)
{
    currentNote = midiNoteNumber;
    velocity = newVelocity;

    oscillator.setFrequency (juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber));
    oscillator.reset();

    envelope.reset();
    envelope.noteOn();
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

    currentNote = -1;
    velocity = 0.0f;
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

void Voice::renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    if (!isVoiceActive())
        return;

    const auto numChannels = buffer.getNumChannels();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto env = envelope.getNextSample();
        const auto value = oscillator.getNextSample() * env * velocity;

        for (int channel = 0; channel < numChannels; ++channel)
            buffer.addSample (channel, startSample + sample, value);
    }

    if (!envelope.isActive())
        currentNote = -1;
}
