#include "Synthesiser.hpp"

void Synthesiser::addVoice(std::unique_ptr<SynthesiserVoice> voice)
{
    if (voice != nullptr)
        voices_.push_back(std::move(voice));
}

void Synthesiser::prepare(double sampleRate)
{
    for (auto& voice : voices_)
        voice->setCurrentPlaybackSampleRate(sampleRate);
}

void Synthesiser::startNote(int midiNoteNumber, float velocity)
{
    if (auto* voice = findFreeVoice())
        voice->startNote(midiNoteNumber, velocity, &sound_, 0);
}

void Synthesiser::stopNote(int midiNoteNumber)
{
    if (auto* voice = findVoiceForNote(midiNoteNumber))
        voice->stopNote(0.0f, false);
}

void Synthesiser::stopAllNotes()
{
    for (auto& voice : voices_)
        if (voice->isVoiceActive())
            voice->stopNote(0.0f, false);
}

SynthesiserVoice* Synthesiser::findFreeVoice()
{
    for (auto& voice : voices_)
        if (!voice->isVoiceActive())
            return voice.get();

    return voices_.empty() ? nullptr : voices_.front().get();
}

SynthesiserVoice* Synthesiser::findVoiceForNote(int midiNoteNumber)
{
    for (auto& voice : voices_)
        if (voice->isVoiceActive() && voice->getCurrentNote() == midiNoteNumber)
            return voice.get();

    return nullptr;
}

void Synthesiser::renderVoices(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    for (auto& voice : voices_)
        voice->renderNextBlock(buffer, startSample, numSamples);
}

void Synthesiser::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    buffer.clear();

    int currentSample = 0;

    for (const auto metadata : midiMessages)
    {
        const auto eventPosition = juce::jlimit(0, buffer.getNumSamples(), metadata.samplePosition);

        if (eventPosition > currentSample)
        {
            renderVoices(buffer, currentSample, eventPosition - currentSample);
            currentSample = eventPosition;
        }

        const auto message = metadata.getMessage();

        if (message.isNoteOn())
            startNote(message.getNoteNumber(), message.getFloatVelocity());
        else if (message.isNoteOff())
            stopNote(message.getNoteNumber());
        else if (message.isAllNotesOff() || message.isAllSoundOff())
            stopAllNotes();
    }

    if (currentSample < buffer.getNumSamples())
        renderVoices(buffer, currentSample, buffer.getNumSamples() - currentSample);
}
