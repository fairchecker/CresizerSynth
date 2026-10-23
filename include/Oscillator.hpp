#pragma once

#include <JuceHeader.h>
#include <memory>

#include "Waveform.hpp"

class Oscillator {
public:
    Oscillator();

    void prepare(double sampleRate);
    void reset() { phase_ = 0.0; }
    void setFrequency(double newFrequency);
    void setWaveform(std::unique_ptr<Waveform> newWaveform);
    float getNextSample();
    void processBlock(juce::AudioBuffer<float>& buffer);

private:
    double phase_ = 0.0;
    double sample_rate_ = 44100.0;
    double frequency_ = 440.0;
    double phase_increment_ = 0.0;
    std::unique_ptr<Waveform> waveform_;

    void updatePhaseIncrement();
};
