#pragma once

class Waveform {
public:
    virtual ~Waveform() = default;
    virtual double calculate(double phase) const = 0;
}