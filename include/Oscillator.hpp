#pragma once

#include <JuceHeader.h>
#include <array>
#include <memory>

#include "Waveform.hpp"

class Oscillator {
public:
    Oscillator();

    void prepare(double sampleRate);
    void reset() { phase_ = 0.0; }
    void setFrequency(double newFrequency);
    /** Realtime-safe: switches waveform without allocation or phase reset. */
    void setWaveform(WaveformType newWaveformType);
    WaveformType getWaveform() const noexcept { return currentWaveformType_; }
    float getNextSample();

private:
    double phase_ = 0.0;
    double sample_rate_ = 44100.0;
    double frequency_ = 440.0;
    double phase_increment_ = 0.0;

    std::array<std::unique_ptr<Waveform>, 4> waveforms_;
    WaveformType currentWaveformType_ = WaveformType::sine;

    void updatePhaseIncrement();
};
