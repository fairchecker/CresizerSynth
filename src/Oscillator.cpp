#include "Oscillator.hpp"

#include "Sinewave.hpp"

Oscillator::Oscillator()
    : waveform_(std::make_unique<Sinewave>())
{
    updatePhaseIncrement();
}

void Oscillator::prepare(double sampleRate)
{
    sample_rate_ = sampleRate;
    updatePhaseIncrement();
}

void Oscillator::setFrequency(double frequency)
{
    frequency_ = frequency;
    updatePhaseIncrement();
}

void Oscillator::setWaveform(std::unique_ptr<Waveform> newWaveform)
{
    if (newWaveform != nullptr)
        waveform_ = std::move(newWaveform);
}

void Oscillator::updatePhaseIncrement()
{
    phase_increment_ = (juce::MathConstants<double>::twoPi * frequency_) / sample_rate_;
}

float Oscillator::getNextSample()
{
    const double value = waveform_ != nullptr ? waveform_->calculate(phase_) : 0.0;

    phase_ += phase_increment_;
    if (phase_ >= juce::MathConstants<double>::twoPi)
        phase_ -= juce::MathConstants<double>::twoPi;

    return static_cast<float>(value);
}

void Oscillator::processBlock(juce::AudioBuffer<float>& buffer)
{
    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = buffer.getNumChannels();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto value = getNextSample();

        for (int channel = 0; channel < numChannels; ++channel)
            buffer.setSample(channel, sample, value);
    }
}
