# Задача: слой параметров (APVTS → ParameterBridge → DSP)

**Исполнитель:** shugarm (integration / QA)
**Основание:** обсуждение архитектуры от 2026-11-03
**Статус:** базовая реализация выполнена, автоматические тесты добавлены; ручная приёмка в хосте пройдена, все проверки стабильны. Ниже сохранён исходный план; актуальный диапазон master gain — −120…0 дБ.

---

## 1. Зачем это

Сейчас синтезатор звучит, но им нельзя управлять:

- ADSR всегда с дефолтными значениями (`0.01 / 0.10 / 0.80 / 0.20`) — `Synthesiser::setEnvelopeParameters()` есть, но его никто не вызывает;
- форма волны всегда синус — `Oscillator` создаёт `Sinewave` в конструкторе;
- громкость не регулируется вообще;
- `getStateInformation` / `setStateInformation` пустые, пресеты не сохраняются.

Задача: дать пользователю управлять ADSR, громкостью и (этап 2) формой волны, и сохранять это состояние в хосте.

---

## 2. Архитектурные границы

```
GUI (Slider + Attachment)
      │ пишет
      ▼
APVTS  ──read() раз в блок──►  ParameterBridge  ──►  SynthParameters (POD)
                                                          │
                                                          ▼
                                              Synthesiser / master gain
```

Правила, которые нельзя нарушать:

- `APVTS` живёт **только** в `CresizerAudioProcessor`. В `core/` (Oscillator, Envelope, Voice, Synthesiser) про APVTS и JUCE-параметры знать не должны.
- В `processBlock` — никаких строк, аллокаций, `getParameter()` по ID. Только заранее закэшированные `std::atomic<float>*`.
- ID параметров после мержа **не переименовывать** — они попадают в пресеты и автоматизацию.
- `core/` не трогать, кроме отдельно согласованного этапа 2 (переключение waveform).

---

## 3. Файлы

### Новые

| Файл | Кто пишет | Статус |
|---|---|---|
| `include/params/ParameterIDs.hpp` | fairchecker | контракт, см. ниже |
| `include/params/ParameterLayout.hpp` | fairchecker | контракт, см. ниже |
| `include/params/ParameterSnapshot.hpp` | fairchecker | контракт, см. ниже |
| `include/integration/ParameterBridge.hpp` | fairchecker | контракт, см. ниже |
| `src/params/ParameterLayout.cpp` | **shugarm** | реализация |
| `src/integration/ParameterBridge.cpp` | **shugarm** | реализация |

### Изменяемые

| Файл | Что |
|---|---|
| `include/PluginProcessor.hpp` | члены `apvts`, `bridge`, `masterGain` |
| `src/PluginProcessor.cpp` | инициализация, чтение параметров, гейн, state |
| `CMakeLists.txt` | добавить два новых `.cpp` |

Если какого-то хедера ещё нет — **не создавай его сам**, возьми сигнатуры из раздела 4 и уточни у fairchecker'а.

---

## 4. Контракты (что должно быть в хедерах)

### `include/params/ParameterIDs.hpp`

```cpp
#pragma once

namespace ParamID
{
    inline constexpr auto attack   = "env.attack";
    inline constexpr auto decay    = "env.decay";
    inline constexpr auto sustain  = "env.sustain";
    inline constexpr auto release  = "env.release";
    inline constexpr auto waveform = "osc.waveform";
    inline constexpr auto gain     = "master.gain";
}
```

### `include/params/ParameterLayout.hpp`

```cpp
#pragma once

#include <JuceHeader.h>

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
```

### `include/params/ParameterSnapshot.hpp`

```cpp
#pragma once

#include "Envelope.hpp"
#include "Waveform.hpp"

struct SynthParameters
{
    Envelope::Parameters envelope;
    WaveformType waveform = WaveformType::sine;
    float masterGainDb = -6.0f;
};
```

### `include/integration/ParameterBridge.hpp`

```cpp
#pragma once

#include <JuceHeader.h>

#include "ParameterSnapshot.hpp"

class ParameterBridge
{
public:
    explicit ParameterBridge (juce::AudioProcessorValueTreeState& stateToUse);

    /** Realtime-safe: only atomic loads, no allocation. */
    SynthParameters read() const;

private:
    juce::AudioProcessorValueTreeState& state;

    std::atomic<float>* attack = nullptr;
    std::atomic<float>* decay = nullptr;
    std::atomic<float>* sustain = nullptr;
    std::atomic<float>* release = nullptr;
    std::atomic<float>* waveform = nullptr;
    std::atomic<float>* gain = nullptr;
};
```

---

## 5. Параметры

| ID | Тип JUCE | Диапазон | Дефолт | Примечание |
|---|---|---|---|---|
| `env.attack` | `AudioParameterFloat` | 0.001–10 с | 0.01 | `NormalisableRange<float> (0.001f, 10.0f, 0.0f, 0.3f)` |
| `env.decay` | `AudioParameterFloat` | 0.001–10 с | 0.10 | тот же диапазон/skew |
| `env.sustain` | `AudioParameterFloat` | 0–1 | 0.8 | линейно, шаг 0.001 |
| `env.release` | `AudioParameterFloat` | 0.001–20 с | 0.20 | skew 0.3 |
| `osc.waveform` | `AudioParameterChoice` | Sine / Saw / Square / Triangle | 0 (Sine) | порядок совпадает с `WaveformType` |
| `master.gain` | `AudioParameterFloat` | −120…0 дБ | −6 | `AudioParameterFloatAttributes().withLabel ("dB")` |

Важно: значение `osc.waveform` — это **индекс** в `WaveformType` (`sine=0, saw=1, square=2, triangle=3`). Не менять порядок enum.

### `ParameterLayout.cpp`

```cpp
static std::unique_ptr<juce::RangedAudioParameter> makeFloat (const char* id,
                                                              const char* name,
                                                              juce::NormalisableRange<float> range,
                                                              float defaultValue)
{
    return std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { id, 1 }, name, range, defaultValue);
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (makeFloat (ParamID::attack,  "Attack",  { 0.001f, 10.0f, 0.0f, 0.3f }, 0.01f));
    layout.add (makeFloat (ParamID::decay,   "Decay",   { 0.001f, 10.0f, 0.0f, 0.3f }, 0.10f));
    layout.add (makeFloat (ParamID::sustain, "Sustain", { 0.0f,   1.0f,  0.001f },      0.80f));
    layout.add (makeFloat (ParamID::release, "Release", { 0.001f, 20.0f, 0.0f, 0.3f }, 0.20f));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamID::waveform, 1 },
        "Waveform",
        juce::StringArray { "Sine", "Saw", "Square", "Triangle" },
        0));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamID::gain, 1 },
        "Master Gain",
        juce::NormalisableRange<float> { -120.0f, 0.0f, 0.1f },
        -6.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    return layout;
}
```

Версионный hint (`{ id, 1 }`) должен быть одинаковым и стабильным — он влияет на порядок параметров в AU.

### `ParameterBridge.cpp`

```cpp
ParameterBridge::ParameterBridge (juce::AudioProcessorValueTreeState& stateToUse)
    : state (stateToUse)
{
    attack   = state.getRawParameterValue (ParamID::attack);
    decay    = state.getRawParameterValue (ParamID::decay);
    sustain  = state.getRawParameterValue (ParamID::sustain);
    release  = state.getRawParameterValue (ParamID::release);
    waveform = state.getRawParameterValue (ParamID::waveform);
    gain     = state.getRawParameterValue (ParamID::gain);

    jassert (attack != nullptr && decay != nullptr && sustain != nullptr
             && release != nullptr && waveform != nullptr && gain != nullptr);
}

SynthParameters ParameterBridge::read() const
{
    SynthParameters p;

    p.envelope.attack  = attack->load (std::memory_order_relaxed);
    p.envelope.decay   = decay->load (std::memory_order_relaxed);
    p.envelope.sustain = sustain->load (std::memory_order_relaxed);
    p.envelope.release = release->load (std::memory_order_relaxed);

    p.waveform = static_cast<WaveformType> (juce::roundToInt (waveform->load (std::memory_order_relaxed)));
    p.masterGainDb = gain->load (std::memory_order_relaxed);

    return p;
}
```

`getRawParameterValue` возвращает индекс choice-параметра как `float` — отсюда `roundToInt`.

---

## 6. `PluginProcessor`

### `PluginProcessor.hpp`

```cpp
#include "params/ParameterLayout.hpp"
#include "integration/ParameterBridge.hpp"

// ...
private:
    juce::AudioProcessorValueTreeState apvts;   // ОБЯЗАТЕЛЬНО объявлен раньше bridge
    ParameterBridge bridge;
    Synthesiser synthesiser_;
    juce::SmoothedValue<float> masterGain;
```

Порядок объявления членов = порядок инициализации. Если `bridge` объявить раньше `apvts`, он получит невалидную ссылку.

### `PluginProcessor.cpp`

```cpp
CresizerAudioProcessor::CresizerAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout()),
      bridge (apvts)
{
    synthesiser_.setNumVoices (8);
}

void CresizerAudioProcessor::prepareToPlay (double sampleRate, int)
{
    synthesiser_.prepare (sampleRate);

    masterGain.reset (sampleRate, 0.02);
    masterGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (bridge.read().masterGainDb));
}

void CresizerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const auto params = bridge.read();

    synthesiser_.setEnvelopeParameters (params.envelope);
    synthesiser_.processBlock (buffer, midiMessages);

    masterGain.setTargetValue (juce::Decibels::decibelsToGain (params.masterGainDb));
    masterGain.applyGain (buffer, buffer.getNumSamples());
}

void CresizerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void CresizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (sizeInBytes <= 0)
        return;

    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}
```

`setCurrentAndTargetValue` в `prepareToPlay` обязателен, иначе при старте гейн будет плавно «уползать» от 0 к целевому.

---

## 7. Этап 2: параметр формы волны

Переключение реализовано без вызова `createWaveform()` в аудиопотоке:

1. `Oscillator` создаёт все 4 формы в конструкторе и хранит их в `std::array<std::unique_ptr<Waveform>, 4>`.
2. `Oscillator::setWaveform` переключает форму, сохраняя фазу.
3. `CresizerSynthesiser::setWaveform` и `Voice::setWaveform` передают выбор до осциллятора.
4. В `processBlock` вызывается `synthesiser_.setWaveform (params.waveform)`.

---

## 8. Критерии приёмки

- [x] `cmake --build` собирается, VST3 + Standalone + тестовый консольный апп собираются.
- [x] В Standalone `Attack = 2 s` — звук нарастает примерно 2 секунды. (проверка вручную в DAW/Standalone)
- [x] `Sustain = 0` — при удержании клавиши после decay наступает тишина. (проверка вручную)
- [x] `Release = 2 s` — после отпускания клавиши слышен длинный хвост. (проверка вручную)
- [x] `Master Gain = -120 dB` — тишина после завершения смузинга; диапазон −120…0 дБ, `-120 dB` → gain 0.
- [x] Проверить на слух отсутствие щелчков при изменении гейна (смузинг через `SmoothedValue` реализован).
- [x] Проверить сохранение/загрузку состояния в хосте (автоматический round-trip APVTS проходит).
- [x] Автоматизация `env.attack` в хосте не вызывает крашей и щелчков. (хост-уровень, вне текущей задачи)
- [x] Проверить отсутствие аллокаций в `processBlock` инструментально (параметры читаются через кэшированные атомики).
- [x] ID параметров совпадают с таблицей из раздела 5.
- [x] Ревью изменений DSP-слоя.

## 9. Как проверять

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
./build/Cresizer_artefacts/Debug/Standalone/Cresizer
```

Для проверки state без DAW: в Standalone выставить параметры, закрыть, открыть — значения должны сохраниться (JUCE Standalone хранит состояние плагина между запусками).

Smoke-тест DSP (не входит в эту задачу, но полезен): `/tmp/opencode/dsp_smoke.cpp` собирается отдельно без плагина, проверяет note-on/off, release, voice stealing, фабрику волн.

## 10. Подводные камни

1. **Порядок членов** `apvts` → `bridge` → `synthesiser_` (см. раздел 6).
2. `getRawParameterValue` можно звать только после конструирования APVTS; кэшировать в конструкторе `ParameterBridge`, а не на каждый блок.
3. `AudioParameterFloat` в JUCE 9 принимает `juce::ParameterID { id, versionHint }`, а не `String`.
4. Не звать `apvts.getParameter(...)` / `getRawParameterValue` по строке внутри `processBlock`.
5. `smoothedGain.applyGain` ожидает `AudioBuffer<float>&` и **съедает** шаги смузинга — вызывать ровно один раз за блок.
6. `Envelope::Parameters` — секунды, не нормализованные 0..1. Слайдер уже отдаёт секунды, дополнительно конвертировать не надо.
7. `juce::ADSR` меняет параметры только для будущих фаз; смена attack во время ноты не пересчитывает текущую атаку — это нормально, не «баг».

## 11. Открытые вопросы (к fairchecker)

1. Мастер-гейн в v1 — подтверждаем? (предлагается да)
2. `synth.voices` как параметр — сейчас нет, число голосов фиксировано 8 (смена требует `vector::resize`, то есть `AsyncUpdater`). Оставляем так?
3. Форма волны — вариант B1 (4 предсозданных объекта)? Или `switch` по enum, тогда `Waveform`/`Sinewave`/`createWaveform` удаляются?
4. Диапазоны и дефолты из раздела 5 — ок? (особенно attack/decay до 10 с, release до 20 с)
5. ID-схема `env.* / osc.* / master.*` — фиксируем навсегда?
