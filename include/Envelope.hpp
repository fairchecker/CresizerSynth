#pragma once

#include <JuceHeader.h>

/** ADSR envelope.

    Times are in seconds, sustain is a level in [0, 1].
*/
class Envelope
{
public:
    struct Parameters
    {
        float attack  = 0.01f;
        float decay   = 0.10f;
        float sustain = 0.80f;
        float release = 0.20f;
    };

    Envelope() { setParameters (parameters); }

    /** Called once before rendering; sets the rate used by juce::ADSR. */
    void prepare (double sampleRate);

    /** Applies new ADSR values. Safe to call from the message thread. */
    void setParameters (const Parameters& newParameters);

    void noteOn()  { adsr.noteOn(); }
    void noteOff() { adsr.noteOff(); }
    void reset()   { adsr.reset(); }

    bool  isActive() const  { return adsr.isActive(); }
    float getNextSample()   { return adsr.getNextSample(); }

private:
    juce::ADSR adsr;
    Parameters parameters;
};
