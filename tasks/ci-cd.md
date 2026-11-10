# CI/CD Pipeline Design

**Status:** Design phase. Local CTest integration for `CresizerTests` is implemented and passing; CI workflow (`.github/workflows/ci.yml`), packaging and release automation are still pending.

---

## 1. Goals

- Automate build, test, and release for **Linux**, **macOS**, **Windows**.
- Produce **VST3** and **Standalone** artefacts.
- Run **CTest** suite on every PR and on merge to `main`.
- Publish versioned releases (GitHub Releases + optional marketplace upload).

---

## 2. Pipeline Stages

| Stage | Runner | Key Steps |
|-------|--------|-----------|
| **configure** | ubuntu-latest, macos-latest, windows-latest | `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release` |
| **build** | same | `cmake --build build -j` |
| **test** | same | `ctest --test-dir build --output-on-failure` |
| **package** | same | Collect `build/Cresizer_artefacts/Release/VST3/`, `Standalone/` |
| **release** | ubuntu-latest (manual dispatch) | `gh release create` with artefacts |

---

## 3. Matrix

```yaml
strategy:
  matrix:
    os: [ubuntu-latest, macos-latest, windows-latest]
    include:
      - os: ubuntu-latest
        deps: sudo apt-get update && sudo apt-get install -y libfreetype6-dev libfontconfig1-dev libx11-dev libxext-dev libxi-dev libxinerama-dev libxcursor-dev libxrandr-dev libxcomposite-dev libasound2-dev libjack-jackd2-dev
      - os: macos-latest
        deps: brew install freetype fontconfig
      - os: windows-latest
        deps: # VS Build Tools + Windows SDK assumed
```

---

## 4. CTest Integration

Implemented locally in top-level `CMakeLists.txt` (`BUILD_TESTING` block):

```cmake
include(CTest)
if(BUILD_TESTING)
    add_executable(CresizerTests tests/dsp_tests.cpp)
    target_include_directories(CresizerTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
        ${CMAKE_BINARY_DIR}/Cresizer_artefacts/JuceLibraryCode
    )
    target_link_libraries(CresizerTests PRIVATE Cresizer juce::juce_audio_utils)
    add_test(NAME CresizerTests COMMAND CresizerTests)
endif()
```

Current suite: `tests/dsp_tests.cpp` (DSP layer: oscillator/waveforms, envelope, gain/mute, peak limiting, synthesiser propagation, voice lifecycle).

Tests must:
- Run without GUI / JUCE message thread. (CresizerTests is a plain `add_executable`, no JUCEApplication — satisfied)
- Use only core DSP classes + `ParameterBridge` + `AudioProcessorValueTreeState`.
- Be deterministic, no flaky timing dependencies. (current suite meets this)

---

## 5. Artefact Naming

```
Cresizer-{version}-{os}-{arch}.zip
  ├── VST3/Cresizer.vst3/
  └── Standalone/Cresizer(.exe)
```

---

## 6. Versioning

- Semantic version from `CMakeLists.txt` (`project(Cresizer VERSION 0.1.0)`).
- Git tag `v{major}.{minor}.{patch}` triggers release workflow.

---

## 7. Status Badges

Add to `README.md` once pipeline is live:
- Build status
- Test status
- Latest release

---

## 8. Next Steps (Implementation)

1. ~~Add `enable_testing()` and test targets to `CMakeLists.txt`.~~ **Done locally** (CresizerTests, `BUILD_TESTING`).
2. ~~Write first test cases~~ **Done** (`tests/dsp_tests.cpp`: DSP, parameters and state round-trip).
3. Create `.github/workflows/ci.yml` with matrix above. (**pending**)
4. Add `release.yml` for manual dispatch. (**pending**)
5. Verify on all three OSes. (**pending**)
6. Add badges to README. (**pending**)
