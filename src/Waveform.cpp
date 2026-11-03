#include "Waveform.hpp"

#include "Sinewave.hpp"

#include <cmath>

namespace
{
    constexpr double pi = 3.14159265358979323846;
    constexpr double twoPi = 2.0 * pi;

    class Saw : public Waveform
    {
    public:
        double calculate (double phase) const override
        {
            return 2.0 * (phase / twoPi) - 1.0;
        }
    };

    class Square : public Waveform
    {
    public:
        double calculate (double phase) const override
        {
            return phase < pi ? 1.0 : -1.0;
        }
    };

    class Triangle : public Waveform
    {
    public:
        double calculate (double phase) const override
        {
            return 4.0 * std::abs (phase / twoPi - 0.5) - 1.0;
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
