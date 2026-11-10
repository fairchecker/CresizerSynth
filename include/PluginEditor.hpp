#pragma once

#include "PluginProcessor.hpp"

class CresizerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    CresizerAudioProcessorEditor (CresizerAudioProcessor& p);
    ~CresizerAudioProcessorEditor() override = default;

    void paint (juce::Graphics& g) override;
    void resized() override;

    /** GUI reads/writes parameters through the processor's APVTS. */
    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return processor_.getAPVTS(); }

private:
    juce::AudioProcessorValueTreeState& getAPVTSRef (CresizerAudioProcessor& p) noexcept { return p.getAPVTS(); }

    CresizerAudioProcessor& processor_;

    juce::Slider attackSlider_;
    juce::Slider decaySlider_;
    juce::Slider sustainSlider_;
    juce::Slider releaseSlider_;
    juce::Slider gainSlider_;
    juce::ComboBox waveformCombo_;

    std::unique_ptr<juce::SliderParameterAttachment> attackAttachment_;
    std::unique_ptr<juce::SliderParameterAttachment> decayAttachment_;
    std::unique_ptr<juce::SliderParameterAttachment> sustainAttachment_;
    std::unique_ptr<juce::SliderParameterAttachment> releaseAttachment_;
    std::unique_ptr<juce::SliderParameterAttachment> gainAttachment_;
    std::unique_ptr<juce::ComboBoxParameterAttachment> waveformAttachment_;

    void buildControls (juce::AudioProcessorValueTreeState& apvts);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CresizerAudioProcessorEditor)
};
