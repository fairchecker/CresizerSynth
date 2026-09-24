#include "Oscillator.hpp"

void Ocillator::prepare(double sampleRate, int samplesPerBlock){
    sample_rate_ = sampleRate;
    samples_per_block_ = samplesPerBlock;
}

void Oscillator::setFrequency(double frequency){
    frequency_ = frequency;
    updatePhaseIncrement();
}

float Oscillator::getNextSample(){
    phase_ += phase_increment_;
    if (phase >= juce::MathConstants<double>::twoPi)
        phase -= juce::MathConstants<double>::twoPi;
    return waveform_.processPhase(phase_);
}

void Ocillator::processBlock(juce::AudioBuffer<float>& buffer)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] = getNextSample();
        }
    }
}