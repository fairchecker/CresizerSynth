#include "TestUtils.hpp"
#include "Voice.hpp"
#include "Oscillator.hpp"
#include "Synthesiser.hpp"
#include "PluginProcessor.hpp"
#include "params/ParameterIDs.hpp"

#include <JuceHeader.h>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <new>

#if JUCE_MSVC
 #include <malloc.h>
#endif

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
    synth.prepare (48000.0);
    synth.startNote (60, 1.0f);
    synth.startNote (62, 1.0f);
    synth.startNote (64, 1.0f);   // steals 60: fade-out, then 64 starts

    juce::AudioBuffer<float> buffer (1, 512);
    buffer.clear();
    juce::MidiBuffer empty;
    synth.processBlock (buffer, empty);

    assert (synth.findVoiceForNote (60) == nullptr);
    assert (synth.findVoiceForNote (64) != nullptr);
    assert (synth.findVoiceForNote (62) != nullptr);

    // Retrigger keeps the same voice alive instead of cutting it.
    synth.startNote (62, 1.0f);
    assert (synth.findVoiceForNote (62) != nullptr);
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
// Test 7: Soft ceiling is transparent below the knee and bounded above it.
// ============================================================
void testSoftLimit()
{
    printf ("Test 7: Soft ceiling ...\n");

    juce::AudioBuffer<float> buffer (2, 4);
    buffer.clear ();
    buffer.getWritePointer (0)[0] = 0.5f;
    buffer.getWritePointer (0)[1] = -0.6f;
    buffer.getWritePointer (0)[2] = 1.5f;
    buffer.getWritePointer (0)[3] = -3.0f;

    CresizerSynthesiser synth;
    synth.applySoftLimit (buffer);

    const float* data = buffer.getWritePointer (0);
    assert (std::abs (data[0] - 0.5f) < 1e-6f);
    assert (std::abs (data[1] - (-0.6f)) < 1e-6f);
    assert (data[2] > 0.7f && data[2] < 0.95f);
    assert (data[3] <= -0.7f && data[3] >= -0.95f - 1e-4f);
    assert (std::abs (data[3]) > std::abs (data[2]));

    // Idempotent and bounded: repeated passes never exceed the ceiling.
    synth.applySoftLimit (buffer);
    assert (std::abs (data[2]) < 0.95f);
    assert (std::abs (data[3]) < 0.95f);

    printf ("  PASS: Soft ceiling\n");
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

// ============================================================
// Test 10: Filter passband/stopband for lowpass and highpass.
// ============================================================
void testFilterFrequencyResponse()
{
    printf ("Test 10: Filter frequency response ...\n");

    constexpr float sampleRate = 48000.0f;
    constexpr int numSamples = 16384;

    auto run = [&] (FilterType type, float cutoffHz, const std::vector<float>& input)
    {
        Filter filter;
        filter.prepare (sampleRate);
        filter.setParameters ({ cutoffHz, 0.0f, type });
        filter.reset();

        std::vector<float> output (input.size());
        for (size_t i = 0; i < input.size(); ++i)
            output[i] = filter.processSample (input[i]);
        return output;
    };

    std::vector<float> lowTone, highTone;
    generateSine (lowTone, 200.0f, sampleRate, numSamples);
    generateSine (highTone, 5000.0f, sampleRate, numSamples);

    const float lowIn = dftMagnitudeAt (lowTone, 200.0f, sampleRate);
    const float highIn = dftMagnitudeAt (highTone, 5000.0f, sampleRate);

    // Lowpass at 500 Hz: 200 Hz passes, 5 kHz is attenuated.
    const auto lpLow = run (FilterType::lowpass, 500.0f, lowTone);
    const auto lpHigh = run (FilterType::lowpass, 500.0f, highTone);
    assert (dftMagnitudeAt (lpLow, 200.0f, sampleRate) > lowIn * 0.7f);
    assert (dftMagnitudeAt (lpHigh, 5000.0f, sampleRate) < highIn * 0.2f);

    // Highpass at 2000 Hz: 200 Hz is attenuated, 5 kHz passes.
    const auto hpLow = run (FilterType::highpass, 2000.0f, lowTone);
    const auto hpHigh = run (FilterType::highpass, 2000.0f, highTone);
    assert (dftMagnitudeAt (hpLow, 200.0f, sampleRate) < lowIn * 0.2f);
    assert (dftMagnitudeAt (hpHigh, 5000.0f, sampleRate) > highIn * 0.7f);

    printf ("  PASS: Filter frequency response\n");
}

// ============================================================
// Test 10b: Disabled filter is a bit-exact bypass.
// ============================================================
void testFilterBypass()
{
    printf ("Test 10b: Filter bypass ...\n");

    Filter filter;
    filter.prepare (48000.0);
    filter.setParameters ({ 300.0f, 0.9f, FilterType::lowpass, false });
    filter.reset();

    for (int i = 0; i < 512; ++i)
    {
        const float input = 0.3f * std::sin (2.0f * juce::MathConstants<float>::pi * 4000.0f
                                             * static_cast<float> (i) / 48000.0f);
        assert (filter.processSample (input) == input);
    }

    // Re-enabling starts from clean state, not the old tail.
    filter.setParameters ({ 300.0f, 0.9f, FilterType::lowpass, true });
    assert (std::isfinite (filter.processSample (1.0f)));

    printf ("  PASS: Filter bypass\n");
}

// ============================================================
// Test 11: Resonance raises the response at the cutoff frequency.
// ============================================================
void testFilterResonancePeak()
{
    printf ("Test 11: Filter resonance peak ...\n");

    constexpr float sampleRate = 48000.0f;
    constexpr int numSamples = 16384;

    std::vector<float> tone;
    generateSine (tone, 1000.0f, sampleRate, numSamples);

    auto magnitudeAtCutoff = [&] (float resonance)
    {
        Filter filter;
        filter.prepare (sampleRate);
        filter.setParameters ({ 1000.0f, resonance, FilterType::lowpass });
        filter.reset();

        std::vector<float> output (tone.size());
        for (size_t i = 0; i < tone.size(); ++i)
            output[i] = filter.processSample (tone[i]);
        return dftMagnitudeAt (output, 1000.0f, sampleRate);
    };

    const float flat = magnitudeAtCutoff (0.0f);      // Q = 0.707 -> gain 0.707 at fc
    const float resonant = magnitudeAtCutoff (0.9f);  // Q ~ 18 -> gain ~18 at fc

    printf ("  flat = %f, resonant = %f\n", flat, resonant);
    assert (resonant > flat * 3.0f);
    assert (resonant < 30.0f);

    printf ("  PASS: Filter resonance peak\n");
}

// ============================================================
// Test 12: DC response matches the filter type.
// ============================================================
void testFilterDcResponse()
{
    printf ("Test 12: Filter DC response ...\n");

    auto dcOutput = [] (FilterType type)
    {
        Filter filter;
        filter.prepare (48000.0);
        filter.setParameters ({ 1000.0f, 0.0f, type });
        filter.reset();

        float output = 0.0f;
        for (int i = 0; i < 48000; ++i)
            output = filter.processSample (1.0f);
        return output;
    };

    assert (std::abs (dcOutput (FilterType::lowpass) - 1.0f) < 0.01f);
    assert (std::abs (dcOutput (FilterType::highpass)) < 0.01f);

    printf ("  PASS: Filter DC response\n");
}

// ============================================================
// Test 13: Stability at cutoff/resonance extremes, every type.
// ============================================================
void testFilterStabilityAtExtremes()
{
    printf ("Test 13: Filter stability at extremes ...\n");

    constexpr float sampleRate = 44100.0f;
    std::vector<float> tone;
    generateSine (tone, 440.0f, sampleRate, 65536);

    for (float cutoff : { 20.0f, 20000.0f })
    {
        for (float resonance : { 0.0f, 1.0f })
        {
            for (FilterType type : { FilterType::lowpass, FilterType::highpass })
            {
                Filter filter;
                filter.prepare (sampleRate);
                filter.setParameters ({ cutoff, resonance, type });
                filter.reset();

                float peak = 0.0f;
                for (float sample : tone)
                {
                    const float out = filter.processSample (sample);
                    assert (std::isfinite (out));
                    peak = std::max (peak, std::abs (out));
                }

                printf ("  cutoff %.0f res %.1f type %d peak = %f\n",
                        cutoff, resonance, static_cast<int> (type), peak);
                assert (peak < 100.0f);
            }
        }
    }

    printf ("  PASS: Filter stability at extremes\n");
}

// ============================================================
// Test 14: Cutoff sweeps stay continuous (no stepped output).
// ============================================================
void testFilterCutoffSweep()
{
    printf ("Test 14: Filter cutoff sweep continuity ...\n");

    constexpr float sampleRate = 48000.0f;
    constexpr int numSamples = 24000;

    std::vector<float> tone;
    generateSine (tone, 200.0f, sampleRate, numSamples, 0.5f);

    Filter filter;
    filter.prepare (sampleRate);
    filter.setParameters ({ 200.0f, 0.3f, FilterType::lowpass });
    filter.reset();

    float previous = 0.0f;
    float maxDelta = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        if (i % 256 == 0)
            filter.setParameters ({ ((i / 256) % 2 == 0) ? 200.0f : 4000.0f, 0.3f, FilterType::lowpass });

        const float out = filter.processSample (tone[i]);
        assert (std::isfinite (out));
        maxDelta = std::max (maxDelta, std::abs (out - previous));
        previous = out;
    }

    printf ("  max sample delta = %f\n", maxDelta);
    assert (maxDelta < 0.25f);

    printf ("  PASS: Filter cutoff sweep continuity\n");
}

// ============================================================
// Test 15: Resonance mapping, APVTS -> bridge -> DSP, state round-trip.
// ============================================================
void testFilterParametersAndState()
{
    printf ("Test 15: Filter parameters, bridge and state ...\n");

    assert (approxEqual (Filter::resonanceToQ (0.0f), 0.7071f, 1e-3f));
    assert (approxEqual (Filter::resonanceToQ (1.0f), 20.0f, 1e-3f));

    CresizerAudioProcessor processor;
    auto& state = processor.getAPVTS();
    auto set = [&] (const char* id, float value)
    {
        auto* parameter = state.getParameter (id);
        assert (parameter != nullptr);
        parameter->setValueNotifyingHost (parameter->convertTo0to1 (value));
    };

    set (ParamID::filterCutoff, 3500.0f);
    set (ParamID::filterResonance, 0.8f);
    set (ParamID::filterType, 1.0f);
    set (ParamID::filterEnabled, 1.0f);

    ParameterBridge bridge (state);
    const auto params = bridge.read();
    assert (std::abs (params.filter.cutoffHz - 3500.0f) < 0.01f);
    assert (std::abs (params.filter.resonance - 0.8f) < 0.001f);
    assert (params.filter.type == FilterType::highpass);
    assert (params.filter.enabled);

    juce::MemoryBlock data;
    processor.getStateInformation (data);
    assert (data.getSize() > 0);

    CresizerAudioProcessor restored;
    restored.setStateInformation (data.getData(), static_cast<int> (data.getSize()));
    assert (std::abs (restored.getAPVTS().getRawParameterValue (ParamID::filterCutoff)->load() - 3500.0f) < 0.01f);
    assert (std::abs (restored.getAPVTS().getRawParameterValue (ParamID::filterResonance)->load() - 0.8f) < 0.001f);
    assert (restored.getAPVTS().getRawParameterValue (ParamID::filterType)->load() == 1.0f);
    assert (restored.getAPVTS().getRawParameterValue (ParamID::filterEnabled)->load() == 1.0f);

    printf ("  PASS: Filter parameters, bridge and state\n");
}

// ============================================================
// Test 16: Filter is applied to the voice mix end to end.
// ============================================================
void testFilterThroughProcessor()
{
    printf ("Test 16: Filter through processor ...\n");

    auto renderNote = [] (float cutoffHz)
    {
        CresizerAudioProcessor processor;
        auto& state = processor.getAPVTS();

        auto* cutoffParam = state.getParameter (ParamID::filterCutoff);
        auto* resonanceParam = state.getParameter (ParamID::filterResonance);
        auto* enabledParam = state.getParameter (ParamID::filterEnabled);
        assert (cutoffParam != nullptr && resonanceParam != nullptr && enabledParam != nullptr);
        cutoffParam->setValueNotifyingHost (cutoffParam->convertTo0to1 (cutoffHz));
        resonanceParam->setValueNotifyingHost (resonanceParam->convertTo0to1 (0.0f));
        enabledParam->setValueNotifyingHost (enabledParam->convertTo0to1 (1.0f));

        processor.prepareToPlay (48000.0, 512);
        juce::AudioBuffer<float> buffer (2, 4096);
        buffer.clear();
        juce::MidiBuffer midi;
        midi.addEvent (juce::MidiMessage::noteOn (1, 84, 1.0f), 0);
        processor.processBlock (buffer, midi);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            for (int n = 0; n < buffer.getNumSamples(); ++n)
                assert (std::isfinite (buffer.getWritePointer (ch)[n]));

        return buffer.getMagnitude (0, buffer.getNumSamples());
    };

    const float closed = renderNote (20.0f);      // 1 kHz note below nothing: killed
    const float open = renderNote (20000.0f);     // 1 kHz note passes

    printf ("  closed = %f, open = %f\n", closed, open);
    assert (open > 0.05f);
    assert (closed < open * 0.1f);

    printf ("  PASS: Filter through processor\n");
}

// ============================================================
// Test 17: processBlock performs no heap allocation.
// ============================================================
namespace
{
    std::atomic<int> allocationCount { 0 };
    std::atomic<bool> countingAllocations { false };
}

void* operator new (std::size_t size)
{
    if (countingAllocations.load (std::memory_order_relaxed))
        allocationCount.fetch_add (1, std::memory_order_relaxed);

    if (auto* memory = std::malloc (size))
        return memory;
    throw std::bad_alloc();
}

void* operator new[] (std::size_t size)
{
    return ::operator new (size);
}

void* operator new (std::size_t size, std::align_val_t alignment)
{
    if (countingAllocations.load (std::memory_order_relaxed))
        allocationCount.fetch_add (1, std::memory_order_relaxed);

   #if JUCE_MSVC
    if (auto* memory = _aligned_malloc (size, static_cast<std::size_t> (alignment)))
        return memory;
   #else
    const auto align = static_cast<std::size_t> (alignment);
    if (auto* memory = std::aligned_alloc (align, (size + align - 1) / align * align))
        return memory;
   #endif

    throw std::bad_alloc();
}

void* operator new[] (std::size_t size, std::align_val_t alignment)
{
    return ::operator new (size, alignment);
}

void operator delete (void* memory) noexcept { std::free (memory); }
void operator delete[] (void* memory) noexcept { std::free (memory); }
void operator delete (void* memory, std::size_t) noexcept { std::free (memory); }
void operator delete[] (void* memory, std::size_t) noexcept { std::free (memory); }

void operator delete (void* memory, std::align_val_t) noexcept
{
   #if JUCE_MSVC
    _aligned_free (memory);
   #else
    std::free (memory);
   #endif
}

void operator delete[] (void* memory, std::align_val_t alignment) noexcept
{
    ::operator delete (memory, alignment);
}

void operator delete (void* memory, std::size_t, std::align_val_t alignment) noexcept
{
    ::operator delete (memory, alignment);
}

void operator delete[] (void* memory, std::size_t, std::align_val_t alignment) noexcept
{
    ::operator delete (memory, alignment);
}

void testNoAllocationInProcessBlock()
{
    printf ("Test 17: No allocation in processBlock ...\n");

    CresizerAudioProcessor processor;
    processor.prepareToPlay (48000.0, 512);

    juce::AudioBuffer<float> buffer (2, 512);
    buffer.clear();
    juce::MidiBuffer midi;
    midi.addEvent (juce::MidiMessage::noteOn (1, 60, 1.0f), 0);
    midi.addEvent (juce::MidiMessage::noteOff (1, 60), 200);

    // Warm up lazy initialisation before measuring.
    processor.processBlock (buffer, midi);

    // Queue a GUI-keyboard note; the measured block must inject it without allocating.
    processor.getKeyboardState().noteOn (1, 64, 1.0f);

    allocationCount.store (0);
    countingAllocations.store (true);
    processor.processBlock (buffer, midi);
    countingAllocations.store (false);

    printf ("  allocations = %d\n", allocationCount.load());
    assert (allocationCount.load() == 0);

    printf ("  PASS: No allocation in processBlock\n");
}

// ============================================================
// Test 18: Default patch is open and an 8-voice chord never hits a hard rail.
// ============================================================
void testDefaultPatchAndHeadroom()
{
    printf ("Test 18: Default patch open, chord under ceiling ...\n");

    CresizerAudioProcessor probe;
    assert (probe.getAPVTS().getRawParameterValue (ParamID::filterEnabled)->load() == 0.0f);
    assert (probe.getAPVTS().getRawParameterValue (ParamID::filterCutoff)->load() > 19999.0f);

    auto render = [] (std::initializer_list<int> notes)
    {
        CresizerAudioProcessor processor;
        auto* waveform = processor.getAPVTS().getParameter (ParamID::waveform);
        assert (waveform != nullptr);
        waveform->setValueNotifyingHost (waveform->convertTo0to1 (1.0f));

        processor.prepareToPlay (48000.0, 512);
        juce::AudioBuffer<float> buffer (1, 8192);
        buffer.clear();
        juce::MidiBuffer midi;
        for (int note : notes)
            midi.addEvent (juce::MidiMessage::noteOn (1, note, 1.0f), 0);
        processor.processBlock (buffer, midi);

        return std::vector<float> (buffer.getReadPointer (0),
                                   buffer.getReadPointer (0) + buffer.getNumSamples());
    };

    const auto single = render ({ 60 });
    const auto chord = render ({ 48, 52, 55, 60, 64, 67, 72, 76 });

    const float singlePeak = findPeak (single);
    float chordPeak = 0.0f;
    for (float sample : chord)
    {
        assert (std::isfinite (sample));
        assert (std::abs (sample) <= 0.95f + 1e-4f);
        chordPeak = std::max (chordPeak, std::abs (sample));
    }

    // Soft ceiling only: no sample sits on a hard rail.
    assert (chordPeak < 0.95f);
    // Voices really sum: a chord is much louder than one note.
    assert (chordPeak > 2.0f * singlePeak);

    printf ("  single peak = %f, chord peak = %f\n", singlePeak, chordPeak);
    printf ("  PASS: Default patch open, chord under ceiling\n");
}

// ============================================================
// Test 19: Retrigger and voice stealing do not step the waveform.
// ============================================================
void testNoClickOnRetriggerOrSteal()
{
    printf ("Test 19: No click on retrigger/steal ...\n");

    CresizerAudioProcessor processor;
    processor.prepareToPlay (48000.0, 512);

    juce::AudioBuffer<float> buffer (1, 16384);
    buffer.clear();

    juce::MidiBuffer midi;
    midi.addEvent (juce::MidiMessage::noteOn (1, 60, 1.0f), 0);
    midi.addEvent (juce::MidiMessage::noteOn (1, 60, 1.0f), 6000);
    const int chord[] = { 62, 64, 65, 67, 69, 71, 72, 74 };
    for (int note : chord)
        midi.addEvent (juce::MidiMessage::noteOn (1, note, 1.0f), 12000);

    processor.processBlock (buffer, midi);

    const float* data = buffer.getReadPointer (0);
    auto maxDelta = [&] (int from, int to)
    {
        float m = 0.0f;
        for (int i = from + 1; i < to; ++i)
            m = std::max (m, std::abs (data[i] - data[i - 1]));
        return m;
    };

    const float steady = maxDelta (2000, 5900);
    const float retrigger = maxDelta (5990, 6010);
    const float steal = maxDelta (11990, 12010);

    printf ("  steady=%f retrigger=%f steal=%f\n", steady, retrigger, steal);
    assert (retrigger < steady * 2.0f + 0.005f);
    assert (steal < steady * 2.0f + 0.005f);

    printf ("  PASS: No click on retrigger/steal\n");
}

// ============================================================
// Test 20: Sweeping gain/cutoff/resonance/type while playing stays continuous.
// ============================================================
void testParameterAutomationIsClickFree()
{
    printf ("Test 20: Parameter automation is click-free ...\n");

    CresizerAudioProcessor processor;
    auto setParam = [&] (const char* id, float value)
    {
        auto* parameter = processor.getAPVTS().getParameter (id);
        assert (parameter != nullptr);
        parameter->setValueNotifyingHost (parameter->convertTo0to1 (value));
    };

    processor.prepareToPlay (48000.0, 512);

    juce::AudioBuffer<float> buffer (1, 512);
    buffer.clear();
    juce::MidiBuffer midi;
    midi.addEvent (juce::MidiMessage::noteOn (1, 60, 1.0f), 0);
    processor.processBlock (buffer, midi);

    float previous = buffer.getReadPointer (0)[buffer.getNumSamples() - 1];
    float maxDelta = 0.0f;
    int maxBlock = -1;

    constexpr int numBlocks = 200;
    for (int block = 0; block < numBlocks; ++block)
    {
        const float t = static_cast<float> (block) / static_cast<float> (numBlocks - 1);
        setParam (ParamID::gain, -60.0f + 60.0f * t);
        setParam (ParamID::filterCutoff, 200.0f + 7800.0f * t);
        setParam (ParamID::filterResonance, t);
        setParam (ParamID::filterType, (block / 50) % 2 == 0 ? 0.0f : 1.0f);
        setParam (ParamID::filterEnabled, 1.0f);

        buffer.clear();
        juce::MidiBuffer empty;
        processor.processBlock (buffer, empty);

        const float* data = buffer.getReadPointer (0);
        float blockDelta = 0.0f;
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            assert (std::isfinite (data[i]));
            blockDelta = std::max (blockDelta, std::abs (data[i] - previous));
            previous = data[i];
        }

        if (blockDelta > maxDelta)
        {
            maxDelta = blockDelta;
            maxBlock = block;
        }
    }

    printf ("  max delta = %f at block %d\n", maxDelta, maxBlock);
    assert (maxDelta < 0.02f);

    printf ("  PASS: Parameter automation is click-free\n");
}

// ============================================================
// Test 21: all-notes-off cancels a queued steal.
// ============================================================
void testAllNotesOffCancelsPendingSteal()
{
    printf ("Test 21: All-notes-off cancels pending steal ...\n");

    CresizerSynthesiser synth;
    synth.setNumVoices (2);
    synth.prepare (48000.0);
    synth.startNote (60, 1.0f);
    synth.startNote (62, 1.0f);
    synth.startNote (64, 1.0f);   // queued behind a fade on 60's voice

    synth.stopAllNotes (true);

    juce::AudioBuffer<float> buffer (1, 1024);
    buffer.clear();
    juce::MidiBuffer empty;
    synth.processBlock (buffer, empty);

    assert (synth.findVoiceForNote (64) == nullptr);
    assert (buffer.getMagnitude (0, buffer.getNumSamples()) < 0.05f);

    printf ("  PASS: All-notes-off cancels pending steal\n");
}

// ============================================================
// Test 22: Second-order slope: 12 dB/octave away from the cutoff.
// ============================================================
void testFilterSlope()
{
    printf ("Test 22: Filter slope ...\n");

    constexpr float sampleRate = 48000.0f;
    constexpr int numSamples = 16384;

    auto magnitude = [&] (FilterType type, float cutoffHz, float frequencyHz)
    {
        Filter filter;
        filter.prepare (sampleRate);
        filter.setParameters ({ cutoffHz, 0.0f, type });
        filter.reset();

        std::vector<float> output (numSamples);
        for (int i = 0; i < numSamples; ++i)
            output[i] = filter.processSample (std::sin (2.0f * juce::MathConstants<float>::pi
                                                        * frequencyHz * static_cast<float> (i) / sampleRate));
        return dftMagnitudeAt (output, frequencyHz, sampleRate);
    };

    const float cutoff = 1000.0f;
    const float lowAtCutoff = magnitude (FilterType::lowpass, cutoff, cutoff);
    const float lowAt4x = magnitude (FilterType::lowpass, cutoff, 4.0f * cutoff);
    assert (lowAtCutoff > lowAt4x * 5.0f);      // ideal ratio ~11.3

    const float highAtCutoff = magnitude (FilterType::highpass, cutoff, cutoff);
    const float highAtQuarter = magnitude (FilterType::highpass, cutoff, cutoff / 4.0f);
    assert (highAtCutoff > highAtQuarter * 5.0f);

    printf ("  PASS: Filter slope\n");
}

// ============================================================
// Test 23: Resonance gain at the cutoff rises monotonically with the knob.
// ============================================================
void testFilterResonanceMonotonic()
{
    printf ("Test 23: Resonance monotonic ...\n");

    constexpr float sampleRate = 48000.0f;
    constexpr int numSamples = 16384;
    constexpr float cutoff = 1000.0f;

    auto gainAtCutoff = [&] (float resonance)
    {
        Filter filter;
        filter.prepare (sampleRate);
        filter.setParameters ({ cutoff, resonance, FilterType::lowpass });
        filter.reset();

        std::vector<float> output (numSamples);
        for (int i = 0; i < numSamples; ++i)
            output[i] = filter.processSample (std::sin (2.0f * juce::MathConstants<float>::pi
                                                        * cutoff * static_cast<float> (i) / sampleRate));
        return dftMagnitudeAt (output, cutoff, sampleRate);
    };

    float previous = -1.0f;
    for (float resonance : { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f })
    {
        const float gain = gainAtCutoff (resonance);
        printf ("  res %.2f -> %f\n", resonance, gain);
        assert (gain > previous);
        previous = gain;
    }

    printf ("  PASS: Resonance monotonic\n");
}

// ============================================================
// Test 24: Long noisy run at maximum resonance stays finite and bounded.
// ============================================================
void testFilterLongRunStability()
{
    printf ("Test 24: Filter long-run stability ...\n");

    Filter filter;
    filter.prepare (48000.0);
    filter.setParameters ({ 150.0f, 1.0f, FilterType::lowpass });
    filter.reset();

    std::uint32_t seed = 1;
    float peak = 0.0f;

    for (int i = 0; i < 200000; ++i)
    {
        seed = seed * 1664525u + 1013904223u;
        const float noise = static_cast<float> (seed >> 8) / 8388608.0f - 1.0f;
        const float out = filter.processSample (noise);
        assert (std::isfinite (out));
        peak = std::max (peak, std::abs (out));
    }

    printf ("  peak = %f\n", peak);
    assert (peak < 200.0f);

    printf ("  PASS: Filter long-run stability\n");
}

// ============================================================
// Test 25: Envelope stages: attack peak, sustain level, release time.
// ============================================================
void testEnvelopeStages()
{
    printf ("Test 25: Envelope stages ...\n");

    constexpr double sampleRate = 48000.0;
    Envelope envelope;
    envelope.prepare (sampleRate);
    envelope.setParameters ({ 0.01f, 0.02f, 0.5f, 0.02f });

    envelope.reset();
    envelope.noteOn();

    float attackPeak = 0.0f;
    const int attackSamples = static_cast<int> (0.01 * sampleRate);
    for (int i = 0; i < attackSamples; ++i)
        attackPeak = std::max (attackPeak, envelope.getNextSample());
    assert (attackPeak > 0.9f && attackPeak <= 1.01f);

    float level = 0.0f;
    for (int i = 0; i < static_cast<int> (0.1 * sampleRate); ++i)
        level = envelope.getNextSample();
    assert (std::abs (level - 0.5f) < 0.02f);

    envelope.noteOff();
    int releaseSamples = 0;
    while (envelope.isActive() && releaseSamples < static_cast<int> (0.5 * sampleRate))
    {
        envelope.getNextSample();
        ++releaseSamples;
    }
    assert (!envelope.isActive());
    assert (releaseSamples < static_cast<int> (0.1 * sampleRate));

    printf ("  PASS: Envelope stages\n");
}

// ============================================================
// Test 26: Sine oscillator really runs at the requested frequency.
// ============================================================
void testOscillatorFrequency()
{
    printf ("Test 26: Oscillator frequency ...\n");

    Oscillator osc;
    osc.prepare (48000.0);
    osc.setFrequency (440.0);

    int risingCrossings = 0;
    float previous = osc.getNextSample();
    for (int i = 0; i < 48000; ++i)
    {
        const float sample = osc.getNextSample();
        if (previous <= 0.0f && sample > 0.0f)
            ++risingCrossings;
        previous = sample;
    }

    printf ("  rising crossings = %d\n", risingCrossings);
    assert (std::abs (risingCrossings - 440) <= 2);

    printf ("  PASS: Oscillator frequency\n");
}

// ============================================================
// Test 27: Parameter normalization round-trips and range endpoints.
// ============================================================
void testParameterNormalization()
{
    printf ("Test 27: Parameter normalization ...\n");

    CresizerAudioProcessor processor;
    auto& state = processor.getAPVTS();

    const char* continuous[] = { ParamID::attack, ParamID::decay, ParamID::sustain,
                                 ParamID::release, ParamID::gain, ParamID::filterCutoff,
                                 ParamID::filterResonance };
    for (const char* id : continuous)
    {
        auto* parameter = state.getParameter (id);
        assert (parameter != nullptr);
        const float value = parameter->convertFrom0to1 (0.37f);
        const float roundTrip = parameter->convertTo0to1 (value);
        assert (std::abs (roundTrip - 0.37f) < 0.02f);
    }

    auto* cutoff = state.getParameter (ParamID::filterCutoff);
    assert (std::abs (cutoff->convertFrom0to1 (0.0f) - 20.0f) < 0.01f);
    assert (std::abs (cutoff->convertFrom0to1 (1.0f) - 20000.0f) < 0.5f);

    auto* gain = state.getParameter (ParamID::gain);
    assert (std::abs (gain->convertFrom0to1 (0.0f) - (-120.0f)) < 0.01f);
    assert (std::abs (gain->convertFrom0to1 (1.0f)) < 0.01f);

    auto* enabled = state.getParameter (ParamID::filterEnabled);
    assert (enabled->convertFrom0to1 (0.0f) == 0.0f);
    assert (enabled->convertFrom0to1 (1.0f) == 1.0f);

    printf ("  PASS: Parameter normalization\n");
}

// ============================================================
// Test 28: Every waveform is DC-free over a full second.
// ============================================================
void testWaveformDcOffset()
{
    printf ("Test 28: Waveform DC offset ...\n");

    constexpr int numSamples = 48000;
    for (int type = 0; type < 4; ++type)
    {
        Oscillator osc;
        osc.prepare (48000.0);
        osc.setFrequency (110.0);
        osc.setWaveform (static_cast<WaveformType> (type));

        double sum = 0.0;
        for (int i = 0; i < numSamples; ++i)
            sum += osc.getNextSample();

        const double mean = sum / numSamples;
        printf ("  waveform %d mean = %f\n", type, mean);
        assert (std::abs (mean) < 0.02);
    }

    printf ("  PASS: Waveform DC offset\n");
}

// ============================================================
// Test 29: Triangle reaches Nyquist instead of stopping at 63 harmonics.
// ============================================================
void testTriangleBandwidth()
{
    printf ("Test 29: Triangle bandwidth ...\n");

    constexpr float sampleRate = 48000.0f;
    constexpr int numSamples = 32768;
    constexpr float fundamental = 130.81f;

    Oscillator osc;
    osc.prepare (sampleRate);
    osc.setFrequency (fundamental);
    osc.setWaveform (WaveformType::triangle);

    std::vector<float> samples (numSamples);
    for (int i = 0; i < numSamples; ++i)
        samples[i] = osc.getNextSample();

    const float f0 = dftMagnitudeAt (samples, fundamental, sampleRate);
    const float h65 = dftMagnitudeAt (samples, fundamental * 65.0f, sampleRate);
    const float h129 = dftMagnitudeAt (samples, fundamental * 129.0f, sampleRate);

    printf ("  h65 rel = %f, h129 rel = %f\n", h65 / f0, h129 / f0);
    assert (h65 > f0 * 1.0e-4f);   // 1/k^2 at k=65 is about 2.4e-4
    assert (h129 > f0 * 2.0e-5f);

    printf ("  PASS: Triangle bandwidth\n");
}

// ============================================================
// Test 35: MidiKeyboardState notes reach the synthesizer.
// ============================================================
void testKeyboardStateThroughProcessor()
{
    printf ("Test 35: Keyboard state reaches the synthesizer ...\n");

    CresizerAudioProcessor processor;
    processor.prepareToPlay (48000.0, 512);

    juce::AudioBuffer<float> buffer (2, 512);
    juce::MidiBuffer midi;

    processor.getKeyboardState().noteOn (1, 60, 1.0f);
    buffer.clear();
    processor.processBlock (buffer, midi);
    const float held = buffer.getMagnitude (0, buffer.getNumSamples());
    assert (held > 0.001f);

    processor.getKeyboardState().noteOff (1, 60, 0.0f);
    float released = held;
    for (int block = 0; block < 64; ++block)
    {
        buffer.clear();
        midi.clear();
        processor.processBlock (buffer, midi);
        released = buffer.getMagnitude (0, buffer.getNumSamples());
    }

    printf ("  held = %f, released = %f\n", held, released);
    assert (released < held * 0.01f);

    printf ("  PASS: Keyboard state reaches the synthesizer\n");
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
    testSoftLimit ();
    testWaveformPropagationThroughSynth ();
    testVoiceStartStopCycle ();
    testStateAndParameters ();
    testVoiceStealingAndRetrigger ();
    testSawAliasing ();
    testWaveformSwitchAndRelease ();
    testFilterFrequencyResponse ();
    testFilterBypass ();
    testFilterResonancePeak ();
    testFilterDcResponse ();
    testFilterStabilityAtExtremes ();
    testFilterCutoffSweep ();
    testFilterParametersAndState ();
    testFilterThroughProcessor ();
    testNoAllocationInProcessBlock ();
    testDefaultPatchAndHeadroom ();
    testNoClickOnRetriggerOrSteal ();
    testParameterAutomationIsClickFree ();
    testAllNotesOffCancelsPendingSteal ();
    testFilterSlope ();
    testFilterResonanceMonotonic ();
    testFilterLongRunStability ();
    testEnvelopeStages ();
    testOscillatorFrequency ();
    testParameterNormalization ();
    testWaveformDcOffset ();
    testTriangleBandwidth ();
    testKeyboardStateThroughProcessor ();

    printf ("\n=== All tests completed ===\n");
    return 0;
}
