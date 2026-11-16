#pragma once

/** Resonant state-variable filter (TPT, topology-preserving transform).

    One core provides lowpass and highpass outputs. Cutoff and resonance are
    ramped internally, so parameter moves do not step the coefficients.

    Math: g = tan(pi * cutoffHz / sampleRate), k = 1 / Q. The TPT integrator
    keeps the response stable for any positive g, including fast sweeps.
*/
enum class FilterType { lowpass, highpass };

class Filter
{
public:
    struct Parameters
    {
        float cutoffHz  = 1000.0f;   // [20, 20000]; clamped to 0.49 * sampleRate
        float resonance = 0.0f;      // [0, 1] -> Q in [0.7071, 20]
        FilterType type = FilterType::lowpass;
        bool enabled = true;         // false = bypass, output equals input

        bool operator== (const Parameters& other) const noexcept
        {
            return cutoffHz == other.cutoffHz
                && resonance == other.resonance
                && type == other.type
                && enabled == other.enabled;
        }

        bool operator!= (const Parameters& other) const noexcept
        {
            return !(*this == other);
        }
    };

    /** Maps the normalized resonance control to the filter's Q factor. */
    static float resonanceToQ (float resonance) noexcept;

    /** Called once before rendering; sets the ramp rate for the current rate. */
    void prepare (double newSampleRate);

    /** Clears the integrator state and snaps the ramped coefficients to target. */
    void reset() noexcept;

    /** Realtime-safe: stores the new targets, no allocation. */
    void setParameters (const Parameters& newParameters) noexcept;
    const Parameters& getParameters() const noexcept { return parameters_; }

    /** Realtime-safe, allocation-free per-sample processing. */
    float processSample (float input) noexcept;

private:
    double sampleRate_ = 44100.0;
    Parameters parameters_;

    // Ramped SVF coefficients: g = tan(pi * fc / fs), k = 1 / Q.
    double targetG_ = 0.0, currentG_ = 0.0;
    double targetK_ = 1.0, currentK_ = 1.0;
    double rampCoeff_ = 1.0;

    // Ramped lowpass <-> highpass crossfade: 0 = lowpass, 1 = highpass.
    double targetMix_ = 0.0, currentMix_ = 0.0;

    // Ramped wet/dry crossfade: 0 = bypass, 1 = filtered.
    double targetWet_ = 1.0, currentWet_ = 1.0;

    // TPT integrator state.
    double ic1eq_ = 0.0, ic2eq_ = 0.0;

    // Coefficients derived from currentG_/currentK_; refreshed when they move.
    double a1_ = 0.0, a2_ = 0.0, a3_ = 0.0;

    void updateTargets() noexcept;
    void snapCoefficients() noexcept;
    void refreshCoefficients() noexcept;
};
