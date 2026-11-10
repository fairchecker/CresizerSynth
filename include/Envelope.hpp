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

        bool operator== (const Parameters& other) const noexcept
        {
            return attack == other.attack
                && decay == other.decay
                && sustain == other.sustain
                && release == other.release;
        }

        bool operator!= (const Parameters& other) const noexcept
        {
            return !(*this == other);
        }
    };

    Envelope() { adsr.setParameters ({ parameters.attack, parameters.decay, parameters.sustain, parameters.release }); }

    /** Called once before rendering; sets the rate used by juce::ADSR. */
    void prepare (double sampleRate);

    /** Apply on the audio thread, or while rendering is stopped. */
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
