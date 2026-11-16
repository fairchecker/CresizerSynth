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
    double calculate (double phase, double increment) override
    {
        const auto t = phase / twoPi;
        return 2.0 * t - 1.0 - polyBlep (t, increment / twoPi);
    }
};

class Square final : public Waveform
{
public:
    double calculate (double phase, double increment) override
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
    void prepare (double sampleRate) override
    {
        dcOutCoeff_ = 1.0 - std::exp (-twoPi * 5.0 / sampleRate);
    }

    double calculate (double phase, double increment) override
    {
        if (increment <= 0.0)
            return 4.0 * std::abs (phase / twoPi - 0.5) - 1.0;

        // Triangle is the integral of the band-limited square, so it inherits the
        // full bandwidth (1/k^2 rolloff) instead of a fixed harmonic count:
        // T(t) = 1 - (2/pi) * integral(square dphi).
        if (! initialized_)
        {
            // Start from the naive triangle at the current phase, so switching
            // waveform mid-note does not step the output.
            const auto naive = 4.0 * std::abs (phase / twoPi - 0.5) - 1.0;
            state_ = (pi * 0.5) * (1.0 - naive);
            initialized_ = true;
            lastPhase_ = phase;
            return naive;
        }

        if (lastPhase_ >= 0.0 && phase < lastPhase_)
        {
            // Sampled edges are not exactly cycle-aligned; use the previous
            // full cycle's mean so the integrator cannot drift.
            if (cyclesSeen_ > 0 && cycleCount_ > 0)
                bias_ = cycleSum_ / static_cast<double> (cycleCount_);

            cycleSum_ = 0.0;
            cycleCount_ = 0;
            ++cyclesSeen_;
        }
        lastPhase_ = phase;

        const auto shifted = std::fmod (phase - pi * 0.5 + twoPi, twoPi);
        const auto square = square_.calculate (shifted, increment);

        cycleSum_ += square;
        ++cycleCount_;
        state_ += (square - bias_) * increment;

        const auto raw = 1.0 - (2.0 / pi) * state_;
        dcOut_ += (raw - dcOut_) * dcOutCoeff_;   // removes any constant offset
        return std::clamp (raw - dcOut_, -1.0, 1.0);
    }

    void reset() override
    {
        state_ = 0.0;
        bias_ = 0.0;
        cycleSum_ = 0.0;
        cycleCount_ = 0;
        cyclesSeen_ = 0;
        lastPhase_ = -1.0;
        dcOut_ = 0.0;
        initialized_ = false;
    }

private:
    Square square_;
    double state_ = 0.0;
    double bias_ = 0.0;
    double cycleSum_ = 0.0;
    int cycleCount_ = 0;
    int cyclesSeen_ = 0;
    double lastPhase_ = -1.0;
    double dcOut_ = 0.0;
    double dcOutCoeff_ = 0.001;
    bool initialized_ = false;
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
