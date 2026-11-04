#include "Synthesiser.hpp"

void Synthesiser::setNumVoices (int count)
{
    voices.clear();
    voices.resize (static_cast<size_t> (juce::jmax (0, count)));

    for (auto& voice : voices)
        voice.prepare (sampleRate);
}

void Synthesiser::prepare (double newSampleRate)
{
    sampleRate = newSampleRate;

    for (auto& voice : voices)
        voice.prepare (sampleRate);
}

void Synthesiser::setEnvelopeParameters (const Envelope::Parameters& newParameters)
{
    for (auto& voice : voices)
        voice.setEnvelopeParameters (newParameters);
}

void Synthesiser::startNote (int midiNoteNumber, float velocity)
{
    if (auto* voice = findFreeVoice())
        voice->startNote (midiNoteNumber, velocity);
}

void Synthesiser::stopNote (int midiNoteNumber)
{
    if (auto* voice = findVoiceForNote (midiNoteNumber))
        voice->stopNote (0.0f, true);
}

void Synthesiser::stopAllNotes (bool allowTailOff)
{
    for (auto& voice : voices)
        if (voice.isVoiceActive())
            voice.stopNote (0.0f, allowTailOff);
}

Voice* Synthesiser::findFreeVoice()
{
    for (auto& voice : voices)
        if (!voice.isVoiceActive())
            return &voice;

    if (voices.empty())
        return nullptr;

    // Voice stealing: reuse the oldest voice. startNote() resets its state, so
    // the stolen note's later note-off simply won't match it any more.
    return &voices.front();
}

Voice* Synthesiser::findVoiceForNote (int midiNoteNumber)
{
    for (auto& voice : voices)
        if (voice.isVoiceActive() && voice.getCurrentNote() == midiNoteNumber)
            return &voice;

    return nullptr;
}

void Synthesiser::renderVoices (juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    for (auto& voice : voices)
        voice.renderNextBlock (buffer, startSample, numSamples);
}

void Synthesiser::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
            stopNote (message.getNoteNumber());
        else if (message.isAllSoundOff())
            stopAllNotes (false);
        else if (message.isAllNotesOff())
            stopAllNotes (true);
    }

    if (currentSample < buffer.getNumSamples())
        renderVoices (buffer, currentSample, buffer.getNumSamples() - currentSample);
}
