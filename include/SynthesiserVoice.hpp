#pragma once

#include <JuceHeader.h>

#include "Oscillator.hpp"

class SynthesiserVoice : public juce::SynthesiserVoice
{
public:
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound,
                   int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
    void setCurrentPlaybackSampleRate(double newRate) override;

    bool isVoiceActive() const override;
    int getCurrentNote() const noexcept { return currentNote_; }

private:
    Oscillator oscillator_;
    Envelope envelope_;
    Envelope::Parameters envParams_;
    float velocity_ = 0.0f;
    int currentNote_ = -1;
};
