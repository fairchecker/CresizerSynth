#include "Sinewave.hpp"

#include <cmath>

double Sinewave::calculate(double phase) const
{
    return std::sin(phase);
}
