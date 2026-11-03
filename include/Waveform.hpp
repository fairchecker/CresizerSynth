#pragma once

#include <memory>

class Waveform {
public:
    virtual ~Waveform() = default;
    virtual double calculate(double phase) const = 0;
};

enum class WaveformType { sine, saw, square, triangle };

std::unique_ptr<Waveform> createWaveform(WaveformType type);

