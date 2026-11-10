#pragma once

#include <memory>

class Waveform {
public:
    virtual ~Waveform() = default;

    /** Calculate waveform sample at given phase.
        @param phase        Current phase in [0, 2*pi)
        @param phaseIncrement Phase increment per sample (2*pi * freq / sampleRate).
                               If 0, no anti-aliasing is applied.
    */
    virtual double calculate (double phase, double phaseIncrement = 0.0) const = 0;
};

enum class WaveformType { sine, saw, square, triangle };

std::unique_ptr<Waveform> createWaveform (WaveformType type);

