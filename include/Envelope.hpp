#pragma once

class Envelope {
    public:
    struct Parameters {
        float attack = 0.01f;
        float decay = 0.10f;
        float sustain = 0.80f;
        float release = 0.20f;
    }

    void prepare(double sampleRate);
    void setParameters(const Parameter&);
    void noteOn() { adsr_.noteOn(); }
    void noteOff() { adsr_.noteOff(); }
    void reset() { adsr_.reset(); }
    bool isActive() const { return adsr_.isActive(); }
    float getNextSample() { return adsr_.getNextSample(); }

    private:
    juce::ADSR adsr_;
    juce::ADSR::Parameters params_;
}