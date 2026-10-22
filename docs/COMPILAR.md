# Instro Natural 0.3

JUCE VST3/Standalone instrument with sfizz sample playback, asynchronous SFZ loading, a factory catalog, MIDI keyboard and the original synthetic engine. Source license: AGPL-3.0-or-later. Samples are separately licensed CC0.

## Build

Use Visual Studio 2022 Desktop C++ tools and CMake 3.22 or newer on Windows x64. The included source archives in Dependencies avoid network fetching of JUCE/sfizz. CMake automatically verifies their SHA256 hashes.

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --target Instro_VST3 Instro_Standalone
```

Expected binaries: `build/Instro_artefacts/Release/VST3/Instro Natural.vst3` and `build/Instro_artefacts/Release/Standalone/Instro Natural.exe`. Place the standalone executable beside a `Banks` folder or select Banks from its interface. Only a bank directory containing `catalog.json` is accepted by the factory browser.

## Checks

```powershell
cmake -S . -B build -A x64 -DINSTRO_BUILD_TESTS=ON
cmake --build build --config Release --target InstroChecks
build/InstroChecks_artefacts/Release/InstroChecks.exe "C:/absolute/path/to/Banks" "C:/absolute/path/to/editor.png"
```

The checks cover natural audio at 44.1/48 kHz with 64/1024/4096-frame blocks, saved state, failed-load recovery, synthetic choices and editor rendering. The factory-bank tests and provenance in the parent Validation folder describe the independent SFZ data tests. See the Portuguese user guide for known mapping limits.

The Windows binaries were built with a portable MSVC 19.44 toolchain using the static C++ runtime. All build dependencies and license notices are included. CMake prefers bundled archives; online URLs are fallbacks.
