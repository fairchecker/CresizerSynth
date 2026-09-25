# CresizerSynth

**CresizerSynth** — учебный VSTi-синтезатор на C++ с использованием [JUCE](https://github.com/juce-framework/JUCE).

Проект разрабатывается как командный проект с разделением на **DSP/backend**, **GUI/JUCE** и **integration/testing**.

---

## Project structure

Проект разделён на три основных слоя:

```text
                         ┌──────────────────────┐
                         │       JUCE / UI      │
                         │      GUI layer       │
                         └──────────┬───────────┘
                                    │
                              parameters
                                    │
                         ┌──────────▼───────────┐
                         │     Integration      │
                         │ parameter/state sync │
                         │      audio bridge    │
                         └──────────┬───────────┘
                                    │
                         ┌──────────▼───────────┐
                         │      DSP / Core      │
                         │  synthesis & maths   │
                         │  audio processing    │
                         └──────────────────────┘
```

### Core / DSP

Отвечает за непосредственно синтез и обработку аудиосигнала:

* осцилляторы;
* генерацию waveform;
* envelope;
* фильтры;
* эффекты;
* математические функции;
* обработку аудиобуфера;
* внутренние DSP-компоненты.

Этот слой по возможности не должен зависеть от JUCE GUI.

### JUCE / GUI

Отвечает за взаимодействие с пользователем и интеграцию с JUCE:

* интерфейс плагина;
* knobs/sliders/buttons;
* визуальное оформление;
* `AudioProcessor`;
* `AudioProcessorEditor`;
* взаимодействие с host;
* MIDI/audio input/output;
* JUCE-specific code.

### Integration / Testing

Отвечает за соединение DSP и GUI:

* передача параметров между интерфейсом и DSP;
* синхронизация состояния;
* связывание UI-контролов с параметрами;
* проверка корректности взаимодействия компонентов;
* integration tests;
* поиск и воспроизведение багов;
* проверка сборки и поведения плагина.

---

# Team

| Участник        | Основная зона ответственности                          |
| --------------- | ------------------------------------------------------ |
| **fairchecker** | Backend, DSP, основная математика, базовый фреймворк   |
| **okeshiiish**    | Frontend, GUI, JUCE и взаимодействие с JUCE API        |
| **shugarm**     | Integration, соединение frontend/backend, тестирование |

### fairchecker — Backend / DSP

Отвечает за:

* архитектуру core/DSP части;
* основные математические алгоритмы;
* генерацию и обработку аудиосигнала;
* DSP-компоненты;
* базовые интерфейсы между DSP и остальными частями проекта;
* общую backend-архитектуру.

### okeshiiish — Frontend / JUCE

Отвечает за:

* GUI;
* визуальный дизайн;
* компоненты интерфейса;
* работу с `AudioProcessorEditor`;
* JUCE-specific implementation;
* взаимодействие интерфейса с параметрами плагина;
* пользовательское взаимодействие.

### shugarm — Integration / QA

Отвечает за:

* связывание GUI и DSP;
* parameter/state communication;
* integration layer;
* тестирование отдельных компонентов;
* integration testing;
* проверку поведения плагина;
* регрессионное тестирование;
* поиск и фиксацию багов.

---

# Architecture principles

Чтобы проект не превратился в один большой JUCE-класс, придерживаемся нескольких принципов.

### 1. DSP не зависит от GUI

DSP-компоненты должны по возможности работать независимо от интерфейса.

```text
GUI
 │
 ▼
Parameters
 │
 ▼
DSP
 │
 ▼
Audio
```

GUI не должен напрямую управлять внутренними состояниями DSP-компонентов.

### 2. JUCE-specific code изолируется

Код, необходимый исключительно для работы с JUCE, желательно держать на границе системы.

Это позволяет:

* проще тестировать DSP;
* уменьшить связанность;
* менять UI без переписывания DSP;
* лучше понимать архитектуру проекта.

### 3. Parameters — единый источник состояния

Каждый пользовательский параметр должен иметь определённое представление в integration layer.

Например:

```text
Oscillator
├── waveform
├── frequency
└── amplitude

Filter
├── cutoff
├── resonance
└── type

Envelope
├── attack
├── decay
├── sustain
└── release
```

UI изменяет параметр → integration layer передаёт его DSP → DSP использует актуальное значение.

### 4. Компоненты должны быть максимально независимыми

Предпочтительно:

```text
Oscillator
Filter
Envelope
Effect
    ↓
Synthesizer
    ↓
AudioProcessor
```

а не:

```text
AudioProcessor
 ├── oscillator code
 ├── filter code
 ├── envelope code
 ├── UI logic
 ├── parameter logic
 └── everything else
```

---

# Suggested directory structure

По мере роста проекта структуру можно привести примерно к следующему виду:

```text
CresizerSynth/
│
├── include/
│   ├── core/
│   │   ├── Oscillator.h
│   │   ├── Filter.h
│   │   ├── Envelope.h
│   │   └── Synthesizer.h
│   │
│   ├── integration/
│   │   └── Parameters.h
│   │
│   └── ui/
│       ├── MainComponent.h
│       ├── Knob.h
│       └── ...
│
├── src/
│   ├── core/
│   │   ├── Oscillator.cpp
│   │   ├── Filter.cpp
│   │   ├── Envelope.cpp
│   │   └── Synthesizer.cpp
│   │
│   ├── integration/
│   │   ├── Parameters.cpp
│   │   └── ...
│   │
│   ├── PluginProcessor.cpp
│   ├── PluginProcessor.h
│   ├── PluginEditor.cpp
│   └── PluginEditor.h
│
├── tests/
│   ├── core/
│   └── integration/
│
├── JUCE/
├── CMakeLists.txt
└── README.md
```

Это **целевая структура**, а не требование реализовать всё сразу.

---

# Roadmap

## Phase 0 — Project foundation

**Цель:** получить стабильный минимальный проект, который собирается у всех участников.

* [x] Создать репозиторий
* [x] Подключить JUCE
* [x] Настроить CMake
* [ ] Зафиксировать используемую версию JUCE
* [ ] Проверить сборку на всех рабочих окружениях
* [ ] Определить базовую архитектуру
* [ ] Определить правила взаимодействия между слоями

---

## Phase 1 — DSP foundation

**Цель:** получить минимальный работающий звуковой движок без сложного GUI.

* [ ] Базовый audio processing pipeline
* [ ] Oscillator
* [ ] Основные waveform
* [ ] Frequency / amplitude control
* [ ] Envelope
* [ ] Базовый filter
* [ ] Стабильная обработка audio buffer
* [ ] Проверка DSP-компонентов независимо от GUI

**Результат:**

```text
MIDI / parameters
        ↓
   Synthesizer
        ↓
      Audio
```

---

## Phase 2 — Parameter system

**Цель:** определить единый способ управления состоянием синтезатора.

* [ ] Описать все основные параметры
* [ ] Определить диапазоны параметров
* [ ] Нормализация параметров
* [ ] Parameter → DSP communication
* [ ] Parameter → GUI communication
* [ ] Синхронизация GUI и внутреннего состояния
* [ ] Подготовить систему сохранения состояния

---

## Phase 3 — GUI

**Цель:** создать полноценный интерфейс синтезатора.

* [ ] Основной layout
* [ ] Oscillator controls
* [ ] Envelope controls
* [ ] Filter controls
* [ ] Master controls
* [ ] Knobs/sliders
* [ ] Value display
* [ ] Visual feedback
* [ ] Обработка пользовательского ввода
* [ ] Привязка UI к parameter system

---

## Phase 4 — Integration

**Цель:** полностью связать интерфейс, параметры и DSP.

```text
┌──────────┐
│    GUI   │
└────┬─────┘
     │
     ▼
┌──────────────┐
│  Parameters  │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│     DSP      │
└──────┬───────┘
       │
       ▼
    Audio
```

* [ ] GUI → parameters
* [ ] Parameters → DSP
* [ ] MIDI → synthesizer
* [ ] Audio → host
* [ ] Проверка automation
* [ ] Проверка state save/load
* [ ] Устранение race conditions и проблем синхронизации
* [ ] Проверка поведения в реальном host

---

## Phase 5 — Effects & advanced DSP

**Цель:** расширить звуковые возможности синтезатора.

Конкретный набор эффектов определяется после завершения базового синтеза.

Возможные направления:

* [ ] LFO
* [ ] Additional oscillators
* [ ] Multiple filter modes
* [ ] Distortion
* [ ] Delay
* [ ] Reverb
* [ ] Chorus
* [ ] Additional modulation
* [ ] Polyphony
* [ ] Unison
* [ ] Portamento

---

## Phase 6 — Testing

**Цель:** убедиться, что изменение одного компонента не ломает остальные.

### Unit tests

Проверять отдельно:

* oscillator;
* envelope;
* filters;
* parameter mapping;
* utility functions;
* DSP mathematical functions.

### Integration tests

Проверять:

* GUI → parameter;
* parameter → DSP;
* MIDI → synthesizer;
* DSP → audio output;
* state save/load;
* plugin initialization.

### Manual testing

Проверять плагин в реальном DAW:

* загрузка;
* MIDI input;
* automation;
* preset/state handling;
* стабильность;
* отсутствие audible artifacts;
* корректность UI.

---

## Phase 7 — Polish & release

**Цель:** превратить рабочий прототип в законченную версию.

* [ ] Оптимизация DSP
* [ ] Проверка CPU usage
* [ ] Проверка memory usage
* [ ] Обработка edge cases
* [ ] Финальный UI polish
* [ ] Устранение известных багов
* [ ] Документация
* [ ] Release build
* [ ] Проверка на поддерживаемых платформах
* [ ] Создание первого релиза

---

# Development workflow

Для работы над проектом используем небольшие независимые задачи.

Пример:

```text
Issue
  ↓
Implementation
  ↓
Local testing
  ↓
Pull Request
  ↓
Code review
  ↓
Integration
  ↓
Test
```

### Branches

Рекомендуемый формат:

```text
main
├── feature/dsp-oscillator
├── feature/gui-filter
├── feature/parameter-system
├── fix/filter-crash
└── test/oscillator
```

### Commit messages

Желательно использовать понятные сообщения:

```text
feat: add oscillator
feat: add filter controls
fix: prevent invalid parameter range
test: add oscillator tests
refactor: separate DSP from JUCE
ui: redesign envelope section
build: update CMake configuration
```

---

# Definition of Done

Задача считается завершённой, если:

* код собирается;
* новая функциональность работает;
* существующая функциональность не сломана;
* соответствующие тесты добавлены или обновлены;
* integration layer корректно соединяет компоненты;
* код находится в соответствующем архитектурном слое;
* Pull Request прошёл review.

---

# Current status

> **Project stage:** Early development

На текущем этапе основная задача — сформировать устойчивую архитектуру и минимальный end-to-end pipeline:

```text
MIDI
 ↓
JUCE
 ↓
Parameters
 ↓
DSP
 ↓
Audio
 ↓
Host
```

После того как этот pipeline стабильно работает, функциональность можно наращивать независимо по слоям.

---

# Team

* **fairchecker** — Backend / DSP / Mathematics
Projectshiiish** — Frontend / GUI / JUCE
* **shugarm** — Integration / Testing / QA

---

# Technologies

* **C++**
* **JUCE**
* **CMake**
* **VST3**
* **Git / GitHub**

JUCE используется как основной C++ framework для реализации audio plugin и GUI.

---

# License

See `LICENSE` for details.
