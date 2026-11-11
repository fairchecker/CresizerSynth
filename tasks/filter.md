# Задача: добавление фильтров в синтезатор

**Ветка:** `feature-filter`  
**База:** `feature-ui-piano` (фаза 2 параметров завершена, фаза 3 GUI в работе)  
**Исполнитель:** fairchecker (DSP/backend) с участием okeshiiish (GUI) и shugarm (integration)

## Суть задачи

Реализовать DSP-фильтр и интегрировать его в звуковой конвейер синтезатора, чтобы управление фильтром можно было вывести на UI (см. задачу «Filter controls» в фазе 3, ветка `feature-ui-piano`).

## Требуемое

1. **DSP-фильтр** (`core/`): базовый резонансный фильтр (lowpass / highpass), расчёт коэффициентов по cutoff и resonance, обработка аудиобуфера без аллокаций в `processBlock`.
2. **Параметры фильтра**: добавить в `ParameterLayout` / `ParameterIDs` — `filter.cutoff`, `filter.resonance`, `filter.type` (выбор типа фильтра).
3. **ParameterBridge**: кэшировать атомики фильтра, отдавать через `SynthParameters`.
4. **Интеграция в `Synthesiser`/`Voice`**: передача параметров фильтра до осциллятора/голоса, применение в аудиопотоке.
5. **GUI** (фаза 3, отдельная задача): слайдеры cutoff/resonance и выбор типа фильтра через APVTS attachments.

## Границы

- `core/` не зависит от JUCE GUI; параметры передаются через кэшированные `std::atomic<float>*` (как в `ParameterBridge`).
- Не менять существующие ID параметров (`env.*`, `osc.*`, `master.*`).
- ID новых параметров фиксировать после мержа (попадают в пресеты и автоматизацию).
- Ревью DSP-слоя перед мержа.

## Приёмка (Definition of Done)

- [ ] код собирается (VST3 + Standalone + тесты)
- [ ] фильтр изменяет звук по cutoff/resonance без щелчков
- [ ] параметры фильтра через APVTS → ParameterBridge → DSP
- [ ] состояние фильтра сохраняется/загружается (APVTS round-trip)
- [ ] ревью DSP-слоя пройдено
