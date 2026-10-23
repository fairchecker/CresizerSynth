#include "SynthesiserVoice.hpp"

bool SynthesiserVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return sound != nullptr;
}

void SynthesiserVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*,
                                 int)
{
    currentNote_ = midiNoteNumber;
    oscillator_.setFrequency(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));
    oscillator_.reset();

    gain_.reset(getSampleRate(), 0.01);
    gain_.setCurrentAndTargetValue(0.0f);
    gain_.setTargetValue(velocity);
}

void SynthesiserVoice::stopNote(float, bool)
{
    gain_.setTargetValue(0.0f);
}

void SynthesiserVoice::pitchWheelMoved(int)
{
}

void SynthesiserVoice::controllerMoved(int, int)
{
}

void SynthesiserVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample,
                                       int numSamples)
{
    if (!isVoiceActive())
        return;

    const auto numChannels = outputBuffer.getNumChannels();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto value = static_cast<float>(oscillator_.getNextSample()) * gain_.getNextValue();

        for (int channel = 0; channel < numChannels; ++channel)
            outputBuffer.addSample(channel, startSample + sample, value);
    }

    if (!gain_.isSmoothing() && gain_.getTargetValue() == 0.0f)
        currentNote_ = -1;
}

void SynthesiserVoice::setCurrentPlaybackSampleRate(double newRate)
{
    juce::SynthesiserVoice::setCurrentPlaybackSampleRate(newRate);
    oscillator_.prepare(newRate);
}

bool SynthesiserVoice::isVoiceActive() const
{
    return currentNote_ >= 0;
}
