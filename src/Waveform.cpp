#include "Waveform.hpp"
#include "Sinewave.hpp"

#include <algorithm>
#include <cmath>

namespace
{
constexpr double pi = 3.14159265358979323846;
constexpr double twoPi = 2.0 * pi;

// Polynomial correction around a discontinuity, with phase and width in cycles.
double polyBlep (double phase, double width)
{
    if (width <= 0.0) return 0.0;
    if (phase < width)
    {
        const auto x = phase / width;
        return x + x - x * x - 1.0;
    }
    if (phase > 1.0 - width)
    {
        const auto x = (phase - 1.0) / width;
        return x * x + x + x + 1.0;
    }
    return 0.0;
}

class Saw final : public Waveform
{
public:
    double calculate (double phase, double increment) const override
    {
        const auto t = phase / twoPi;
        return 2.0 * t - 1.0 - polyBlep (t, increment / twoPi);
    }
};

class Square final : public Waveform
{
public:
    double calculate (double phase, double increment) const override
    {
        // Shift by a quarter cycle so phase zero is the centre of the high plateau.
        const auto t = std::fmod (phase / twoPi + 0.25, 1.0);
        const auto width = increment / twoPi;
        const auto otherEdge = std::fmod (t + 0.5, 1.0);
        return (t < 0.5 ? 1.0 : -1.0) + polyBlep (t, width) - polyBlep (otherEdge, width);
    }
};

class Triangle final : public Waveform
{
public:
    double calculate (double phase, double increment) const override
    {
        if (increment <= 0.0)
            return 4.0 * std::abs (phase / twoPi - 0.5) - 1.0;

        // The 1/k^2 coefficients converge quickly; cap work at low frequencies.
        const auto harmonicLimit = std::min (63.0, std::max (0.0, std::ceil (pi / increment) - 1.0));
        double sum = 0.0;
        for (int k = 1; k <= static_cast<int> (harmonicLimit); k += 2)
            sum += std::cos (k * phase) / (static_cast<double> (k) * k);
        return (8.0 / (pi * pi)) * sum;
    }
};
}

std::unique_ptr<Waveform> createWaveform (WaveformType type)
{
    switch (type)
    {
        case WaveformType::sine:     return std::make_unique<Sinewave>();
        case WaveformType::saw:      return std::make_unique<Saw>();
        case WaveformType::square:   return std::make_unique<Square>();
        case WaveformType::triangle: return std::make_unique<Triangle>();
    }
    return std::make_unique<Sinewave>();
}
