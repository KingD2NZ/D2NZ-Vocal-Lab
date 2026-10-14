#include "PluginProcessor.h"
#include <iostream>
#include <stdexcept>

void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
int main() {
    try {
        juce::ScopedJuceInitialiser_GUI gui;
        VocalLabProcessor original;
        original.prepareToPlay(48000, 128);
        original.setCurrentProgram(2);
        auto* output = original.state.getParameter("output");
        output->setValueNotifyingHost(output->convertTo0to1(-6.f));
        juce::MemoryBlock saved;
        original.getStateInformation(saved);
        require(saved.getSize() > 0, "empty project state");
        VocalLabProcessor restored;
        restored.prepareToPlay(48000, 128);
        restored.setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
        require(restored.getCurrentProgram() == 2, "factory program not restored");
        for (const auto* id : {"clean", "power", "de-ess", "space", "echo", "tone", "output"})
            require(original.state.getRawParameterValue(id)->load() == restored.state.getRawParameterValue(id)->load(), "parameter not restored");
        restored.setStateInformation("invalid", 7);
        require(restored.state.getRawParameterValue("output")->load() == -6.f, "bad state must not overwrite settings");
        original.setCurrentProgram(0);
        juce::AudioBuffer<float> audio(2, 128);
        audio.clear(); audio.setSample(0, 0, 0.25f); audio.setSample(1, 0, 0.125f);
        juce::MidiBuffer midi;
        original.processBlock(audio, midi);
        require(std::abs(audio.getSample(0, 0) - 0.25f) < 1.e-6f, "neutral plugin must pass audio");
        require(original.inputPeak.load() == 0.25f, "input meter must reflect audio");
        require(original.getLatencySamples() == 0, "prototype latency incorrect");
        std::cout << "PASS plugin state round-trip, invalid state, preset, real audio and metering\n";
        return 0;
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
