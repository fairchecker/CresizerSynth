#pragma once

#include "PluginProcessor.hpp"

class CresizerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    private juce::Timer
{
public:
    CresizerAudioProcessorEditor (CresizerAudioProcessor& p);
    ~CresizerAudioProcessorEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void parentHierarchyChanged() override;
    void visibilityChanged() override;

    /** GUI reads/writes parameters through the processor's APVTS. */
    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return processor_.getAPVTS(); }

private:
    juce::AudioProcessorValueTreeState& getAPVTSRef (CresizerAudioProcessor& p) noexcept { return p.getAPVTS(); }

    CresizerAudioProcessor& processor_;
    juce::MidiKeyboardComponent keyboardComponent_;

    juce::Slider attackSlider_;
    juce::Slider decaySlider_;
    juce::Slider sustainSlider_;
    juce::Slider releaseSlider_;
    juce::Slider gainSlider_;
    juce::ComboBox waveformCombo_;
    juce::Slider filterCutoffSlider_;
    juce::Slider filterResonanceSlider_;
    juce::ComboBox filterTypeCombo_;
    juce::ToggleButton filterEnabledButton_;

    std::unique_ptr<juce::SliderParameterAttachment> attackAttachment_;
    std::unique_ptr<juce::SliderParameterAttachment> decayAttachment_;
    std::unique_ptr<juce::SliderParameterAttachment> sustainAttachment_;
    std::unique_ptr<juce::SliderParameterAttachment> releaseAttachment_;
    std::unique_ptr<juce::SliderParameterAttachment> gainAttachment_;
    std::unique_ptr<juce::ComboBoxParameterAttachment> waveformAttachment_;
    std::unique_ptr<juce::SliderParameterAttachment> filterCutoffAttachment_;
    std::unique_ptr<juce::SliderParameterAttachment> filterResonanceAttachment_;
    std::unique_ptr<juce::ComboBoxParameterAttachment> filterTypeAttachment_;
    std::unique_ptr<juce::ButtonParameterAttachment> filterEnabledAttachment_;

    void buildControls (juce::AudioProcessorValueTreeState& apvts);
    void requestKeyboardFocus();
    void timerCallback() override;

    bool keyboardFocusGrabbed_ = false;
    int focusAttempts_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CresizerAudioProcessorEditor)
};
