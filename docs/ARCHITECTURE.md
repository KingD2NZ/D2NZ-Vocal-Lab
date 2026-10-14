# Architecture and next milestones

## Boundaries

The initial engine is header-only C++17 and independent of JUCE. `prepare` allocates delay buffers; `process` performs bounded processing without file/network operations or allocations. Mono and stereo are supported. Compression uses a shared detector. Parameters are smoothed over approximately 20 ms.

The JUCE adapter uses AudioProcessorValueTreeState for automation and state persistence. Parameter IDs are stable: clean, power, de-ess, space, echo, tone, output. State schema version is 1. State loading rejects unsupported schemas rather than guessing. Factory presets are Neutral, Street Clean prototype and Velvet prototype. Choosing a program sets actual host-visible parameters. These are starting points, not the final ten commercial presets.

Input/output peaks and gain reduction come from DSP and are published atomically; custom UI rendering must consume those measurements. The current generic editor has no visible meter display. Its controls do process audio.

## Scope of the first engine

There is zero algorithmic lookahead, reported as zero plugin latency. This does not describe DAW/interface monitoring latency. Delay and room tails are processing effects, not compensation latency. The host is told a conservative three-second tail.

This is not yet the complete designed chain: no general parametric EQ or advanced compressor controls; cleanup is a fixed filter mapping. Compression has no knee or automatic makeup gain. Saturation needs oversampling. The comb tail must be replaced by a tuned multi-delay reverb. Host bypass/state/preset transitions need DAW validation and possibly additional fade/reset policies. Automated parameter smoothing does not prove click-free host bypass.

## Milestones

1. Foundation: compile DSP tests and macOS AU/VST3; validate real audio in the two DAWs, automation and project round-trip.
2. DSP quality: general EQ, soft knee, gain matching, improved de-essing, oversampled saturation, proper reverb, tempo-synced filtered/ducked delay. Measure frequency response, gain curves, aliasing, tails, CPU and transitions; run vocal listening comparisons.
3. D2NZ native interface: branded Easy controls, real meters and contextual plots, preset browser. No inactive simulated TUNE knob.
4. Advanced controls: canonical DSP parameters; explicit per-parameter overrides protect edits from macro remapping. Macro values cannot always describe a manually edited module precisely.
5. Learn My Voice: bounded pre-processing capture, background analysis, confidence/insufficient-input handling, Apply/Compare/Cancel and reset. Store features/settings rather than recordings by default.
6. Pitch feasibility: independently compare original and licensable engines; choose using latency, formant quality, tracking and commercial terms. No shipping promises before measured evidence.
7. Beta/release: host matrix, code signing/notarization, installers, documentation and consent-based diagnostics.

## Analysis design

Capture input before this plugin's own DSP; upstream DAW effects may already have processed it. Queue bounded data for a worker, never perform heavy analysis in processBlock. Measure RMS/peaks/crest factor, band energies, sibilance events and F0 only in reliable monophonic voiced passages. Estimate noise only when suitable pauses exist. Recommend bounded adjustments within a preset, allow preview/cancel, and never infer identity or gender. ML is optional after comparison against a deterministic baseline.

## Required macOS validation

Logic Pro AU and FL Studio VST3: 44.1/48/96 kHz, 32/128/512/2048 frames; mono, stereo and mono-to-stereo; automation, bypass, preset changes, project restoration and offline bounce. Include silence, clipping, whisper/rap/sung vocals, long playback and multiple instances. Document DAW/OS/hardware versions and CPU/latency measurements. A macOS CI compile does not substitute for these tests.
