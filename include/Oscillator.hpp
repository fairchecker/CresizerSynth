#pragma once
#include <JuceHeader.h>

class Oscillator {
    public:
    void prepare(double sampleRate, int samplesPerBlock);
    void reset() { phase_ = 0; }
    void setFrequency(float newFrequency){ frequency_ = newFrequency; }
    void setWaveform(Waveform newWaveform) { waveform_ = newWaveform; }
    float getNextSample();
    void processBlock(juce::AudioBuffer<float>& buffer)

    private:
    double phase_;
    double sample_rate_;
    double frequency_;
    double phase_increment_;
    int samples_per_block_;
    Waveform waveform_;
    void updatePhaseIncrement() { phase_increment_ = (juce::MathConstants<double>::twoPi * frequency_) / sample_rate_; }
}