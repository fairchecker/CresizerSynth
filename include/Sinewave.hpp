#pragma once

#include "Waveform.hpp"

class Sinewave : public Waveform {
public:
    double calculate(double phase, double phaseIncrement) const override;
};
