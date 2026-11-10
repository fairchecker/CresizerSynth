#include "TestUtils.hpp"
#include "Voice.hpp"
#include "Oscillator.hpp"
#include "Synthesiser.hpp"
#include "PluginProcessor.hpp"
#include "params/ParameterIDs.hpp"

#include <JuceHeader.h>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>

// Unlike assert(), checks remain active in Release builds.
#undef assert
#define assert(condition) do { if (!(condition)) { std::fprintf (stderr, "FAILED %s:%d: %s\n", __FILE__, __LINE__, #condition); std::abort(); } } while (false)

// ============================================================
// Test 1: Sine wave at phase 0 must be 0.
// ============================================================
void testSineWaveValue()
{
    printf ("Test 1: Sine wave value at phase 0 ...\n");

    Oscillator osc;
    osc.prepare (44100.0);

    assert (osc.getNextSample () == 0.0f);

    printf ("  PASS: Sine wave value at phase 0\n");
}

// ============================================================
// Test 2: PolyBLEP saw is centred at its discontinuity.
// ============================================================
void testSawWaveValue()
{
    printf ("Test 2: Saw wave value at phase 0 ...\n");

    Oscillator osc;
    osc.prepare (44100.0);
    osc.setWaveform (WaveformType::saw);

    assert (std::abs (osc.getNextSample ()) < 0.01f);

    printf ("  PASS: Saw wave value at phase 0\n");
}

// ============================================================
// Test 3: Square wave at phase 0 must be +1, triangle at 0 must be +1.
// ============================================================
void testSquareAndTriangleWaveValues()
{
    printf ("Test 3: Square and Triangle wave values at phase 0 ...\n");

    Oscillator osc;
    osc.prepare (44100.0);

    osc.setWaveform (WaveformType::square);
    // Nyquist-limited harmonic sum approximates the ideal square (value ~1.02 at phase 0).
    assert (std::abs (osc.getNextSample () - 1.0f) < 0.05f);

    osc.setWaveform (WaveformType::triangle);
    // Nyquist-limited harmonic sum approximates the ideal triangle (~0.97 at phase 0).
    assert (std::abs (osc.getNextSample () - 1.0f) < 0.05f);

    printf ("  PASS: Square and Triangle wave values at phase 0\n");
}

// ============================================================
// Test 4: All waveforms bounded and finite.
// ============================================================
void testWaveformsBounded()
{
    printf ("Test 4: Waveform output bounded and finite ...\n");

    Oscillator osc;
    osc.prepare (44100.0);

    for (int type = 0; type < 4; ++type)
    {
        osc.setWaveform (static_cast<WaveformType> (type));

        float maxAbs = 0.0f;
        for (int i = 0; i < 256; ++i)
        {
            const float sample = osc.getNextSample ();
            assert (!std::isnan (sample));
            assert (!std::isinf (sample));
            maxAbs = std::max (maxAbs, std::abs (sample));
        }

        printf ("  waveform %d maxAbs = %f\n", type, maxAbs);
        assert (maxAbs <= 1.0f);
    }

    printf ("  PASS: Waveform output bounded and finite\n");
}

// ============================================================
// Test 5: Envelope parameter change detection (operator== / !=).
// ============================================================
void testEnvelopeParametersChange()
{
    printf ("Test 5: Envelope parameters change detection ...\n");

    Envelope::Parameters params1 { 0.5f, 0.5f, 0.8f, 0.5f };
    Envelope::Parameters params2 { 0.5f, 0.5f, 0.8f, 0.5f };
    Envelope::Parameters params3 { 1.0f, 0.5f, 0.8f, 0.5f };

    assert (params1 == params2);
    assert (params1 != params3);
    assert (!(params1 != params2));

    printf ("  PASS: Envelope parameters change detection\n");
}

// ============================================================
// Test 6: Gain range -120..0 dB, -120 dB must be silence.
// ============================================================
void testGainRange()
{
    printf ("Test 6: Gain range -120..0dB with -120 exact mute ...\n");

    const float gainMinus120 = juce::Decibels::decibelsToGain (-120.0f, -120.0f);
    const float gain0db = juce::Decibels::decibelsToGain (0.0f);
    const float gainMinus60 = juce::Decibels::decibelsToGain (-60.0f);

    assert (gainMinus120 == 0.0f);
    assert (std::abs (gain0db - 1.0f) < 1e-4f);
    assert (std::abs (gainMinus60 - 0.001f) < 1e-4f);

    printf ("  PASS: Gain range -120..0dB with -120 exact mute\n");
}

void testStateAndParameters()
{
    CresizerAudioProcessor original;
    auto& state = original.getAPVTS();
    auto set = [&] (const char* id, float value)
    {
        auto* parameter = state.getParameter (id);
        assert (parameter != nullptr);
        parameter->setValueNotifyingHost (parameter->convertTo0to1 (value));
    };
    set (ParamID::attack, 2.0f);
    set (ParamID::waveform, 2.0f);
    set (ParamID::gain, -120.0f);
    juce::MemoryBlock data;
    original.getStateInformation (data);
    assert (data.getSize() > 0);
    CresizerAudioProcessor restored;
    restored.setStateInformation (data.getData(), static_cast<int> (data.getSize()));
    assert (std::abs (restored.getAPVTS().getRawParameterValue (ParamID::attack)->load() - 2.0f) < 0.001f);
    assert (restored.getAPVTS().getRawParameterValue (ParamID::waveform)->load() == 2.0f);
    assert (restored.getAPVTS().getRawParameterValue (ParamID::gain)->load() == -120.0f);
    juce::MemoryBlock invalidState;
    juce::AudioProcessor::copyXmlToBinary (juce::XmlElement ("OTHER"), invalidState);
    restored.setStateInformation (invalidState.getData(), static_cast<int> (invalidState.getSize()));
    assert (restored.getAPVTS().getRawParameterValue (ParamID::waveform)->load() == 2.0f);
    restored.prepareToPlay (44100.0, 512);
    juce::AudioBuffer<float> audio (2, 512);
    juce::MidiBuffer midi;
    midi.addEvent (juce::MidiMessage::noteOn (1, 60, 1.0f), 0);
    restored.processBlock (audio, midi);
    assert (audio.getMagnitude (0, 512) == 0.0f);
}

void testWaveformSwitchAndRelease()
{
    Oscillator sine, saw;
    sine.prepare (48000.0);
    saw.prepare (48000.0);
    saw.setWaveform (WaveformType::saw);
    float difference = 0.0f;
    for (int i = 0; i < 512; ++i)
        difference += std::abs (sine.getNextSample() - saw.getNextSample());
    assert (difference > 10.0f);

    Voice voice;
    voice.prepare (48000.0);
    voice.setEnvelopeParameters ({ 0.001f, 0.01f, 0.8f, 0.01f });
    voice.startNote (60, 1.0f);
    juce::AudioBuffer<float> buffer (1, 1024);
    buffer.clear();
    voice.renderNextBlock (buffer, 0, 512);
    assert (buffer.getMagnitude (0, 512) > 0.01f);
    voice.stopNote (0.0f, true);
    voice.renderNextBlock (buffer, 0, 1024);
    assert (!voice.isVoiceActive());
}

void testVoiceStealingAndRetrigger()
{
    CresizerSynthesiser synth;
    assert (synth.findFreeVoice() == nullptr);
    synth.setNumVoices (2);
    synth.startNote (60, 1.0f);
    synth.startNote (62, 1.0f);
    synth.startNote (64, 1.0f);
    assert (synth.findVoiceForNote (60) == nullptr);
    assert (synth.findVoiceForNote (62) != nullptr);
    synth.startNote (62, 1.0f);
    synth.stopNote (62, false);
    assert (synth.findVoiceForNote (62) == nullptr);
}

void testSawAliasing()
{
    constexpr int count = 1024;
    constexpr double sampleRate = 48000.0;
    constexpr double frequency = 9000.0;
    Oscillator osc;
    osc.prepare (sampleRate);
    osc.setFrequency (frequency);
    osc.setWaveform (WaveformType::saw);
    std::vector<float> filtered (count), naive (count);
    for (int i = 0; i < count; ++i)
    {
        filtered[i] = osc.getNextSample();
        naive[i] = static_cast<float> (2.0 * std::fmod (i * frequency / sampleRate, 1.0) - 1.0);
    }
    // Third harmonic (27 kHz) folds to 21 kHz at 48 kHz sample rate.
    assert (dftMagnitudeAt (filtered, 21000.0f, 48000.0f) < dftMagnitudeAt (naive, 21000.0f, 48000.0f) * 0.5f);

    osc.reset();
    osc.setWaveform (WaveformType::square);
    for (int i = 0; i < count; ++i)
    {
        filtered[i] = osc.getNextSample();
        const auto phase = std::fmod (i * frequency / sampleRate + 0.25, 1.0);
        naive[i] = phase < 0.5 ? 1.0f : -1.0f;
    }
    assert (dftMagnitudeAt (filtered, 21000.0f, 48000.0f) < dftMagnitudeAt (naive, 21000.0f, 48000.0f) * 0.5f);
}

// ============================================================
// Test 7: Peak limiting guarantees peak <= 1.0.
// ============================================================
void testPeakLimiting()
{
    printf ("Test 7: Peak limiting ...\n");

    juce::AudioBuffer<float> buffer (2, 4);
    buffer.clear ();
    buffer.getWritePointer (0)[0] = 1.5f;
    buffer.getWritePointer (0)[1] = -1.5f;
    buffer.getWritePointer (0)[2] = 0.5f;
    buffer.getWritePointer (0)[3] = -0.5f;

    CresizerSynthesiser synth;
    synth.applyPeakLimit (buffer);

    assert (std::abs (buffer.getWritePointer (0)[0] - 1.0f) < 1e-6f);
    assert (std::abs (buffer.getWritePointer (0)[1] - (-1.0f)) < 1e-6f);
    assert (std::abs (buffer.getWritePointer (0)[2] - 0.5f) < 1e-6f);
    assert (std::abs (buffer.getWritePointer (0)[3] - (-0.5f)) < 1e-6f);

    printf ("  PASS: Peak limiting\n");
}

// ============================================================
// Test 8: Waveform propagates through the synthesiser and stays bounded.
// ============================================================
void testWaveformPropagationThroughSynth()
{
    printf ("Test 8: Waveform propagation through synthesiser ...\n");

    CresizerSynthesiser synth;
    synth.setNumVoices (1);
    synth.prepare (44100.0);
    synth.setWaveform (WaveformType::saw);

    synth.startNote (60, 1.0f);

    juce::AudioBuffer<float> buffer (2, 1024);
    juce::MidiBuffer mid;
    synth.processBlock (buffer, mid);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        for (int n = 0; n < buffer.getNumSamples(); ++n)
        {
            const float v = buffer.getWritePointer (ch)[n];
            assert (!std::isnan (v));
            assert (std::abs (v) <= 1.0f);
        }
    }

    synth.stopAllNotes (true);

    printf ("  PASS: Waveform propagation through synthesiser\n");
}

// ============================================================
// Test 9: Voice start/stop cycle.
// ============================================================
void testVoiceStartStopCycle()
{
    printf ("Test 9: Voice start/stop cycle ...\n");

    Voice voice;
    voice.prepare (44100.0);
    voice.startNote (60, 1.0f);
    assert (voice.isVoiceActive ());
    assert (voice.getCurrentNote () == 60);
    voice.stopNote (0.0f, false);
    assert (!voice.isVoiceActive ());

    voice.startNote (62, 1.0f);
    assert (voice.isVoiceActive ());

    // Tail-off: note stays assigned until the release fades out.
    voice.stopNote (0.0f, true);
    assert (voice.isVoiceActive ());

    // Hard cut: frees the voice immediately.
    voice.stopNote (0.0f, false);
    assert (!voice.isVoiceActive ());

    printf ("  PASS: Voice start/stop cycle\n");
}

int main()
{
    printf ("=== CresizerSynth DSP Tests ===\n\n");

    testSineWaveValue ();
    testSawWaveValue ();
    testSquareAndTriangleWaveValues ();
    testWaveformsBounded ();
    testEnvelopeParametersChange ();
    testGainRange ();
    testPeakLimiting ();
    testWaveformPropagationThroughSynth ();
    testVoiceStartStopCycle ();
    testStateAndParameters ();
    testVoiceStealingAndRetrigger ();
    testSawAliasing ();
    testWaveformSwitchAndRelease ();

    printf ("\n=== All tests completed ===\n");
    return 0;
}
