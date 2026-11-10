#include "Oscillator.hpp"

#include "Waveform.hpp"

Oscillator::Oscillator()
{
    // Precreate all four waveforms for allocation-free realtime-safe switching.
    for (int i = 0; i < static_cast<int> (waveforms_.size ()); ++i)
        waveforms_[i] = createWaveform (static_cast<WaveformType> (i));

    currentWaveformType_ = WaveformType::sine;
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

void Oscillator::updatePhaseIncrement()
{
    phase_increment_ = sample_rate_ > 0.0 && std::isfinite (frequency_)
        ? juce::MathConstants<double>::twoPi * juce::jlimit (0.0, 0.5, frequency_ / sample_rate_)
        : 0.0;
}

void Oscillator::setWaveform(WaveformType newWaveformType)
{
    const auto index = static_cast<int> (newWaveformType);
    if (index >= 0 && index < static_cast<int> (waveforms_.size ()) && waveforms_[index] != nullptr)
        currentWaveformType_ = newWaveformType;
}

float Oscillator::getNextSample()
{
    const WaveformType type = currentWaveformType_;
    const auto index = static_cast<int> (type);
    double value = 0.0;

    if (index >= 0 && index < static_cast<int> (waveforms_.size ()) && waveforms_[index] != nullptr)
        value = waveforms_[index]->calculate (phase_, phase_increment_);

    phase_ += phase_increment_;
    if (phase_ >= juce::MathConstants<double>::twoPi)
        phase_ -= juce::MathConstants<double>::twoPi;

    return static_cast<float> (value);
}
