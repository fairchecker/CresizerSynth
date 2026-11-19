#include "PluginEditor.hpp"

#include "params/ParameterIDs.hpp"

CresizerAudioProcessorEditor::CresizerAudioProcessorEditor (CresizerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor_ (p)
{
    buildControls (getAPVTSRef (processor_));
    setSize (460, 480);
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

    filterCutoffSlider_.setSliderStyle (juce::Slider::LinearHorizontal);
    filterCutoffSlider_.setTextBoxStyle (juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible (filterCutoffSlider_);

    filterResonanceSlider_.setSliderStyle (juce::Slider::LinearHorizontal);
    filterResonanceSlider_.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    addAndMakeVisible (filterResonanceSlider_);

    filterTypeCombo_.addItem ("Lowpass", 1);
    filterTypeCombo_.addItem ("Highpass", 2);
    addAndMakeVisible (filterTypeCombo_);

    filterEnabledButton_.setButtonText ("Enabled");
    addAndMakeVisible (filterEnabledButton_);

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

    if (auto* cutoffParam = apvts.getParameter (juce::String (ParamID::filterCutoff)))
    {
        filterCutoffAttachment_.reset (new juce::SliderParameterAttachment (*cutoffParam, filterCutoffSlider_));
        filterCutoffAttachment_->sendInitialUpdate ();
    }

    if (auto* resonanceParam = apvts.getParameter (juce::String (ParamID::filterResonance)))
    {
        filterResonanceAttachment_.reset (new juce::SliderParameterAttachment (*resonanceParam, filterResonanceSlider_));
        filterResonanceAttachment_->sendInitialUpdate ();
    }

    if (auto* typeParam = apvts.getParameter (juce::String (ParamID::filterType)))
    {
        filterTypeAttachment_.reset (new juce::ComboBoxParameterAttachment (*typeParam, filterTypeCombo_));
        filterTypeAttachment_->sendInitialUpdate ();
    }

    if (auto* enabledParam = apvts.getParameter (juce::String (ParamID::filterEnabled)))
    {
        filterEnabledAttachment_.reset (new juce::ButtonParameterAttachment (*enabledParam, filterEnabledButton_));
        filterEnabledAttachment_->sendInitialUpdate ();
    }
}

void CresizerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);
    g.setColour (juce::Colours::white);
    const char* labels[] = { "Waveform", "Attack", "Decay", "Sustain", "Release", "Master gain",
                             "Filter cutoff", "Filter resonance", "Filter type", "Filter" };
    for (int i = 0; i < 10; ++i)
        g.drawText (labels[i], 12, 12 + i * 40, 100, 32, juce::Justification::centredLeft);

    drawKeyboard (g, { 12, 412, getWidth() - 24, 56 });
}

void CresizerAudioProcessorEditor::drawKeyboard (juce::Graphics& g, juce::Rectangle<int> area)
{
    constexpr int numWhiteKeys = 15;   // C3..C5 inclusive
    constexpr int whiteKeysPerOctave = 7;
    const float whiteWidth = static_cast<float> (area.getWidth()) / static_cast<float> (numWhiteKeys);

    // White keys.
    g.setColour (juce::Colours::white);
    for (int i = 0; i < numWhiteKeys; ++i)
    {
        const juce::Rectangle<float> key (area.getX() + i * whiteWidth,
                                          static_cast<float> (area.getY()),
                                          whiteWidth,
                                          static_cast<float> (area.getHeight()));
        g.fillRect (key.reduced (0.5f));
    }

    g.setColour (juce::Colours::black);
    for (int i = 0; i < numWhiteKeys; ++i)
    {
        const juce::Rectangle<float> key (area.getX() + i * whiteWidth,
                                          static_cast<float> (area.getY()),
                                          whiteWidth,
                                          static_cast<float> (area.getHeight()));
        g.drawRect (key, 1.0f);
    }

    // Black keys sit after white keys C, D, F, G, A of each octave.
    static constexpr int blackAfterWhite[] = { 0, 1, 3, 4, 5 };
    for (int octave = 0; octave < 2; ++octave)
    {
        for (int i = 0; i < 5; ++i)
        {
            const float centreX = area.getX() + (octave * whiteKeysPerOctave + blackAfterWhite[i] + 1) * whiteWidth;
            const juce::Rectangle<float> key (centreX - whiteWidth * 0.3f,
                                              static_cast<float> (area.getY()),
                                              whiteWidth * 0.6f,
                                              area.getHeight() * 0.6f);
            g.fillRect (key);
        }
    }
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
    filterCutoffSlider_.setBounds (112, 252, width, 32);
    filterResonanceSlider_.setBounds (112, 292, width, 32);
    filterTypeCombo_.setBounds (112, 332, width, 32);
    filterEnabledButton_.setBounds (112, 372, 120, 32);
}
