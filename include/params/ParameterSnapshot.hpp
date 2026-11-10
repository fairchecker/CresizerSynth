#pragma once

#include "Envelope.hpp"
#include "Waveform.hpp"

struct SynthParameters
{
    Envelope::Parameters envelope;
    WaveformType waveform = WaveformType::sine;
    float masterGainDb = -6.0f;
};