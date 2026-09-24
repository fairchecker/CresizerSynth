#pragma once

#include "Waveform.hpp"

class Sinewave : public Waveform {
    public double calculate(double phase) const override;
}