#pragma once

#include "Envelope.hpp"
#include "Filter.hpp"
#include "Waveform.hpp"

struct SynthParameters
{
    Envelope::Parameters envelope;
    Filter::Parameters filter;
    WaveformType waveform = WaveformType::sine;
    float masterGainDb = -6.0f;
};