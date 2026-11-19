#include "Filter.hpp"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr double kPi = 3.14159265358979323846;
    constexpr double kRampSeconds = 0.01;
    constexpr double kMinCutoffHz = 1.0;
    constexpr double kMinQ = 0.70710678118654752;   // Butterworth
    constexpr double kMaxQ = 20.0;
}

float Filter::resonanceToQ (float resonance) noexcept
{
    const double clamped = std::clamp (static_cast<double> (resonance), 0.0, 1.0);
    return static_cast<float> (kMinQ + clamped * (kMaxQ - kMinQ));
}

void Filter::prepare (double newSampleRate)
{
    sampleRate_ = std::max (1.0, newSampleRate);

    // One-pole ramp: reaches ~63% of a target step every kRampSeconds.
    rampCoeff_ = 1.0 - std::exp (-1.0 / (kRampSeconds * sampleRate_));

    updateTargets();
    reset();
}

void Filter::reset() noexcept
{
    ic1eq_ = 0.0;
    ic2eq_ = 0.0;
    snapCoefficients();
}

void Filter::setParameters (const Parameters& newParameters) noexcept
{
    if (parameters_ == newParameters)
        return;

    parameters_ = newParameters;
    updateTargets();
}

void Filter::updateTargets() noexcept
{
    const double nyquistGuard = 0.49 * sampleRate_;
    const double cutoff = std::clamp (static_cast<double> (parameters_.cutoffHz),
                                      kMinCutoffHz, nyquistGuard);

    targetG_ = std::tan (kPi * cutoff / sampleRate_);
    targetK_ = 1.0 / static_cast<double> (resonanceToQ (parameters_.resonance));
    targetMix_ = parameters_.type == FilterType::highpass ? 1.0 : 0.0;
    targetWet_ = parameters_.enabled ? 1.0 : 0.0;
}

void Filter::snapCoefficients() noexcept
{
    currentG_ = targetG_;
    currentK_ = targetK_;
    currentMix_ = targetMix_;
    currentWet_ = targetWet_;
    refreshCoefficients();
}

void Filter::refreshCoefficients() noexcept
{
    a1_ = 1.0 / (1.0 + currentG_ * (currentG_ + currentK_));
    a2_ = currentG_ * a1_;
    a3_ = currentG_ * a2_;
}

float Filter::processSample (float input) noexcept
{
    if (currentG_ != targetG_ || currentK_ != targetK_)
    {
        currentG_ += rampCoeff_ * (targetG_ - currentG_);
        currentK_ += rampCoeff_ * (targetK_ - currentK_);
        refreshCoefficients();
    }

    if (currentMix_ != targetMix_)
        currentMix_ += rampCoeff_ * (targetMix_ - currentMix_);

    if (currentWet_ != targetWet_)
        currentWet_ += rampCoeff_ * (targetWet_ - currentWet_);

    const double x = static_cast<double> (input);
    const double v3 = x - ic2eq_;
    const double v1 = a1_ * ic1eq_ + a2_ * v3;
    const double v2 = ic2eq_ + a2_ * ic1eq_ + a3_ * v3;

    ic1eq_ = 2.0 * v1 - ic1eq_;
    ic2eq_ = 2.0 * v2 - ic2eq_;

    const double low = v2;
    const double high = x - currentK_ * v1 - v2;
    const double wet = low + currentMix_ * (high - low);

    // Wet/dry crossfade: settling at 0 gives a bit-exact bypass.
    return static_cast<float> (x + currentWet_ * (wet - x));
}
