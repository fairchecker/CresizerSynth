#include "Sinewave.hpp"

#include <cmath>

double Sinewave::calculate(double phase, double)
{
    return std::sin(phase);
}
