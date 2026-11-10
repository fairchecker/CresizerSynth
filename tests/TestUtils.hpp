#pragma once

#include <JuceHeader.h>
#include <vector>
#include <cmath>
#include <algorithm>

// Test utilities
inline bool approxEqual (float a, float b, float tolerance = 1e-4f)
{
    return std::abs (a - b) <= tolerance;
}

inline bool approxEqual (double a, double b, double tolerance = 1e-6)
{
    return std::abs (a - b) <= tolerance;
}

// Generate a sine wave for spectral analysis
inline void generateSine (std::vector<float>& out, float freq, float sampleRate, int numSamples, float amplitude = 1.0f)
{
    out.resize (static_cast<size_t> (numSamples));
    for (int i = 0; i < numSamples; ++i)
        out[i] = amplitude * std::sin (2.0f * juce::MathConstants<float>::pi * freq * static_cast<float>(i) / sampleRate);
}

// Simple DFT magnitude at a given frequency
inline float dftMagnitudeAt (const std::vector<float>& signal, float freq, float sampleRate)
{
    const int N = static_cast<int> (signal.size());
    float real = 0.0f, imag = 0.0f;
    const float omega = 2.0f * juce::MathConstants<float>::pi * freq / sampleRate;

    for (int n = 0; n < N; ++n)
    {
        real += signal[n] * std::cos (omega * static_cast<float>(n));
        imag += signal[n] * std::sin (omega * static_cast<float>(n));
    }

    return std::sqrt (real * real + imag * imag) / static_cast<float>(N);
}

// Check that a signal has no significant energy above a given frequency
inline bool isBandlimited (const std::vector<float>& signal, float maxFreq, float sampleRate, float threshold = 0.01f)
{
    const int N = static_cast<int> (signal.size());
    // Check frequencies above maxFreq up to Nyquist
    for (float freq = maxFreq + 100.0f; freq < sampleRate / 2.0f; freq += 100.0f)
    {
        const float mag = dftMagnitudeAt (signal, freq, sampleRate);
        if (mag > threshold)
            return false;
    }
    return true;
}

// Find peak amplitude
inline float findPeak (const std::vector<float>& signal)
{
    float peak = 0.0f;
    for (float s : signal)
        peak = std::max (peak, std::abs (s));
    return peak;
}