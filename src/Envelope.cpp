#include "Envelope.hpp"

void Envelope::prepare (double sampleRate)
{
    adsr.setSampleRate (sampleRate);
}

void Envelope::setParameters (const Parameters& newParameters)
{
    parameters = newParameters;

    adsr.setParameters ({ parameters.attack,
                          parameters.decay,
                          parameters.sustain,
                          parameters.release });
}
