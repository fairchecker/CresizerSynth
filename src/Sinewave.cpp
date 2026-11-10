#include "Sinewave.hpp"

#include <cmath>

double Sinewave::calculate(double phase, double) const
{
    return std::sin(phase);
}
