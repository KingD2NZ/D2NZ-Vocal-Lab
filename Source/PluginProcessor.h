#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "dsp/VocalChain.h"
#include <atomic>

class VocalLabProcessor final : public juce::AudioProcessor {
public:
    VocalLabProcessor();
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "D2NZ Vocal Lab"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }
    int getNumPrograms() override { return 3; }
    int getCurrentProgram() override { return program; }
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorValueTreeState state;
    std::atomic<float> inputPeak{0}, outputPeak{0}, gainReduction{0};
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
    d2nz::VocalChain chain;
    std::array<std::atomic<float>*, 7> controls{};
    int program = 0;
};
