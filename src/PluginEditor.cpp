#include "PluginEditor.hpp"

#include "params/ParameterIDs.hpp"

CresizerAudioProcessorEditor::CresizerAudioProcessorEditor (CresizerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor_ (p)
{
    buildControls (getAPVTSRef (processor_));
    setSize (460, 270);
}

void CresizerAudioProcessorEditor::buildControls (juce::AudioProcessorValueTreeState& apvts)
{
    attackSlider_.setSliderStyle (juce::Slider::LinearHorizontal);
    attackSlider_.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    addAndMakeVisible (attackSlider_);

    decaySlider_.setSliderStyle (juce::Slider::LinearHorizontal);
    decaySlider_.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    addAndMakeVisible (decaySlider_);

    sustainSlider_.setSliderStyle (juce::Slider::LinearHorizontal);
    sustainSlider_.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    addAndMakeVisible (sustainSlider_);

    releaseSlider_.setSliderStyle (juce::Slider::LinearHorizontal);
    releaseSlider_.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    addAndMakeVisible (releaseSlider_);

    gainSlider_.setSliderStyle (juce::Slider::LinearHorizontal);
    gainSlider_.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    addAndMakeVisible (gainSlider_);

    waveformCombo_.addItem ("Sine", 1);
    waveformCombo_.addItem ("Saw", 2);
    waveformCombo_.addItem ("Square", 3);
    waveformCombo_.addItem ("Triangle", 4);
    addAndMakeVisible (waveformCombo_);

    if (auto* attackParam = apvts.getParameter (juce::String (ParamID::attack)))
    {
        attackAttachment_.reset (new juce::SliderParameterAttachment (*attackParam, attackSlider_));
        attackAttachment_->sendInitialUpdate ();
    }

    if (auto* decayParam = apvts.getParameter (juce::String (ParamID::decay)))
    {
        decayAttachment_.reset (new juce::SliderParameterAttachment (*decayParam, decaySlider_));
        decayAttachment_->sendInitialUpdate ();
    }

    if (auto* sustainParam = apvts.getParameter (juce::String (ParamID::sustain)))
    {
        sustainAttachment_.reset (new juce::SliderParameterAttachment (*sustainParam, sustainSlider_));
        sustainAttachment_->sendInitialUpdate ();
    }

    if (auto* releaseParam = apvts.getParameter (juce::String (ParamID::release)))
    {
        releaseAttachment_.reset (new juce::SliderParameterAttachment (*releaseParam, releaseSlider_));
        releaseAttachment_->sendInitialUpdate ();
    }

    if (auto* gainParam = apvts.getParameter (juce::String (ParamID::gain)))
    {
        gainAttachment_.reset (new juce::SliderParameterAttachment (*gainParam, gainSlider_));
        gainAttachment_->sendInitialUpdate ();
    }

    if (auto* waveformParam = apvts.getParameter (juce::String (ParamID::waveform)))
    {
        waveformAttachment_.reset (new juce::ComboBoxParameterAttachment (*waveformParam, waveformCombo_));
        waveformAttachment_->sendInitialUpdate ();
    }
}

void CresizerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);
    g.setColour (juce::Colours::white);
    const char* labels[] = { "Waveform", "Attack", "Decay", "Sustain", "Release", "Master gain" };
    for (int i = 0; i < 6; ++i)
        g.drawText (labels[i], 12, 12 + i * 40, 100, 32, juce::Justification::centredLeft);
}

void CresizerAudioProcessorEditor::resized()
{
    const int width = getWidth() - 124;
    waveformCombo_.setBounds (112, 12, width, 32);
    attackSlider_.setBounds (112, 52, width, 32);
    decaySlider_.setBounds (112, 92, width, 32);
    sustainSlider_.setBounds (112, 132, width, 32);
    releaseSlider_.setBounds (112, 172, width, 32);
    gainSlider_.setBounds (112, 212, width, 32);
}
