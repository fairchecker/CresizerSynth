#include "Synthesiser.hpp"

void CresizerSynthesiser::setNumVoices (int count)
{
    voices.clear();
    voices.resize (static_cast<size_t> (juce::jmax (0, count)));

    for (auto& voice : voices)
        voice.prepare (sampleRate);
}

void CresizerSynthesiser::prepare (double newSampleRate)
{
    sampleRate = newSampleRate;

    for (auto& voice : voices)
        voice.prepare (sampleRate);
}

void CresizerSynthesiser::setEnvelopeParameters (const Envelope::Parameters& newParameters)
{
    for (auto& voice : voices)
        voice.setEnvelopeParameters (newParameters);
}

void CresizerSynthesiser::setWaveform (WaveformType newWaveformType)
{
    for (auto& voice : voices)
        voice.setWaveform (newWaveformType);
}

void CresizerSynthesiser::startNote (int midiNoteNumber, float velocity)
{
    // Retriggering a pitch replaces its previous voice, avoiding stuck notes.
    if (auto* existing = findVoiceForNote (midiNoteNumber))
        existing->stopNote (0.0f, false);

    if (auto* voice = findFreeVoice())
    {
        voice->startNote (midiNoteNumber, velocity);
        voice->setStartOrder (++noteCounter);
    }
}

void CresizerSynthesiser::stopNote (int midiNoteNumber, bool allowTailOff)
{
    if (auto* voice = findVoiceForNote (midiNoteNumber))
        voice->stopNote (0.0f, allowTailOff);
}

void CresizerSynthesiser::stopAllNotes (bool allowTailOff)
{
    for (auto& voice : voices)
        if (voice.isVoiceActive())
            voice.stopNote (0.0f, allowTailOff);
}

Voice* CresizerSynthesiser::findFreeVoice()
{
    for (auto& voice : voices)
        if (!voice.isVoiceActive())
            return &voice;

    if (voices.empty())
        return nullptr;

    // Voice stealing: reuse the oldest voice. startNote() resets its state, so
    // the stolen note's later note-off simply won't match it any more.
    return findOldestVoice();
}

Voice* CresizerSynthesiser::findVoiceForNote (int midiNoteNumber)
{
    for (auto& voice : voices)
        if (voice.isVoiceActive() && voice.getCurrentNote() == midiNoteNumber)
            return &voice;

    return nullptr;
}

Voice* CresizerSynthesiser::findOldestVoice()
{
    // Steal the oldest sounding voice (lowest startOrder = longest playing).
    Voice* oldest = nullptr;

    for (auto& voice : voices)
    {
        if (voice.isVoiceActive() && (oldest == nullptr || voice.getStartOrder() < oldest->getStartOrder()))
        {
            oldest = &voice;
        }
    }

    // If no active voice, steal the first one (it's already inactive).
    return oldest != nullptr ? oldest : (voices.empty() ? nullptr : &voices.front());
}

void CresizerSynthesiser::renderVoices (juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    for (auto& voice : voices)
        voice.renderNextBlock (buffer, startSample, numSamples);
}

void CresizerSynthesiser::applyPeakLimit (juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* data = buffer.getWritePointer (channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const float value = data[sample];

            // Explicit limiter/clamp: guarantee peak <= 1.0
            if (value > 1.0f)
                data[sample] = 1.0f;
            else if (value < -1.0f)
                data[sample] = -1.0f;
        }
    }
}

void CresizerSynthesiser::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    buffer.clear();

    int currentSample = 0;

    for (const auto metadata : midiMessages)
    {
        const auto eventPosition = juce::jlimit (0, buffer.getNumSamples(), metadata.samplePosition);

        if (eventPosition > currentSample)
        {
            renderVoices (buffer, currentSample, eventPosition - currentSample);
            currentSample = eventPosition;
        }

        const auto message = metadata.getMessage();

        if (message.isNoteOn())
            startNote (message.getNoteNumber(), message.getFloatVelocity());
        else if (message.isNoteOff())
            stopNote (message.getNoteNumber(), true);
        else if (message.isAllSoundOff())
            stopAllNotes (false);
        else if (message.isAllNotesOff())
            stopAllNotes (true);
    }

    if (currentSample < buffer.getNumSamples())
        renderVoices (buffer, currentSample, buffer.getNumSamples() - currentSample);

    // Explicit limiter/clamp after gain: guarantee peak <= 1.0
    applyPeakLimit (buffer);
}
