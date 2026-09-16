#pragma once
#include "PluginProcessor.hpp"

class CresizerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    CresizerAudioProcessorEditor (CresizerAudioProcessor& p)
        : AudioProcessorEditor (&p)
    {
        setSize (400, 300);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::darkgrey);
        g.setColour (juce::Colours::white);
        g.setFont (15.0f);
        g.drawFittedText ("Hello from Cresizer!", getLocalBounds(),
                          juce::Justification::centred, 1);
    }

    void resized() override {}

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CresizerAudioProcessorEditor)
};