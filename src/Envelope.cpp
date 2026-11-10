#include "Envelope.hpp"

void Envelope::prepare (double sampleRate)
{
    adsr.setSampleRate (sampleRate);
    adsr.setParameters ({ parameters.attack, parameters.decay, parameters.sustain, parameters.release });
}

void Envelope::setParameters (const Parameters& newParameters)
{
    // Only update ADSR when at least one parameter value changes.
    if (parameters != newParameters) {
        parameters = newParameters;
        adsr.setParameters ({ parameters.attack,
                            parameters.decay,
                            parameters.sustain,
                            parameters.release });
    }
}
