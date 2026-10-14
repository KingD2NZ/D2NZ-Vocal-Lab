# D2NZ Vocal Lab

**One Plugin. Your Sound.**

Native C++ vocal processing project for FL Studio and Logic Pro. Target formats: macOS AU and VST3, Apple Silicon first. Windows is a later phase.

## Current milestone: DSP foundation

This is a technical prototype, not a commercial release. The standalone DSP tests run on Linux without JUCE. The JUCE adapter defines AU/VST3 targets, seven real automated parameters, three factory prototype programs and versioned project state. Its temporary parameter editor is functional; the metallic D2NZ UI follows after the DSP milestone.

Implemented processing:

- CLEAN: blended 90 Hz high-pass and broad low-frequency reduction.
- POWER: stereo-linked feed-forward compression, 8 ms attack, 120 ms release, macro-dependent threshold and ratio. No automatic loudness matching yet.
- DE-ESS: simple high-band envelope-controlled attenuation around 5.5 kHz.
- TONE: soft harmonic saturation. No oversampling yet; aliasing still needs measurement and improvement.
- SPACE: damped feedback-comb tail, deliberately an interim algorithm, not a finished plate or professional reverb.
- ECHO: fixed approximately 240 ms delay, 35% feedback. No tempo sync or ducking yet.
- OUTPUT: smoothed -24 to +12 dB gain. No limiter; positive gain can clip.

There is no TUNE control, pitch correction, denoising, voice learning, Record/Mix switch, premium interface, advanced editing or license activation. Preset import/export and user preset management are not implemented yet. No claims of proprietary hardware emulation.

## Build the DSP tests

Requirements: C++17 compiler, CMake >= 3.22.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

The test executable fails nonzero on a failed check, even in Release. It exercises three sample rates, several buffer sizes, gain, transparency, cleanup, dynamics, sibilance attenuation, saturation, tails, reset and stereo linking. Synthetic signal tests are not a substitute for vocal listening tests.

## Build plugins on macOS

Install Xcode command line tools and CMake. Review JUCE's licence before distribution: the code pins JUCE 8.0.6 to commit `51a8a6d7aeae7326956d747737ccf1575e61e209` and downloads it from GitHub when plugin builds are enabled.

```sh
cmake -S . -B build-mac -DD2NZ_BUILD_PLUGIN=ON \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build-mac --parallel 2
ctest --test-dir build-mac --output-on-failure
```

Bundles are generated beneath `build-mac/D2NZVocalLab_artefacts/Release/`. For local testing, copy the `.component` bundle into `~/Library/Audio/Plug-Ins/Components/` and the `.vst3` into `~/Library/Audio/Plug-Ins/VST3/`. Restart/rescan the DAW. Use `auval -v aufx DvLb D2NZ` to validate the AU. Unsigned development bundles are not release installers.

Verify in Logic and FL Studio: mono/stereo input, every control, factory presets, automation, save/reopen, bypass and offline bounce. Actual macOS/DAW tests remain required; a successful Linux DSP test does not validate AU or VST3 hosting.

The macOS CI workflow builds the bundles and uploads development artifacts when pushed. It does not sign, notarize or publish a release.

## Repository organisation

- `Source/dsp/`: host-independent audio processing.
- `Source/PluginProcessor.*`: JUCE host adapter, parameters and state.
- `tests/`: executable DSP regression tests.
- `docs/`: architecture, branding, milestones and validation status.

Development changes are local until explicitly pushed to GitHub. Use the existing isolated checkout; do not create a worktree unless requested.
