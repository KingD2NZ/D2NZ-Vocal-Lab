#include "PluginProcessor.h"

juce::AudioProcessorValueTreeState::ParameterLayout VocalLabProcessor::layout() {
    juce::AudioProcessorValueTreeState::ParameterLayout result;
    for (const auto* name : {"Clean", "Power", "De-ess", "Space", "Echo", "Tone"}) {
        const auto id = juce::String(name).toLowerCase();
        result.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id, 1}, name,
            juce::NormalisableRange<float>(0.f, 1.f), 0.f));
    }
    result.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"output", 1}, "Output (dB)",
        juce::NormalisableRange<float>(-24.f, 12.f), 0.f));
    return result;
}
VocalLabProcessor::VocalLabProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "D2NZState", layout()) {
    int index = 0;
    for (const auto* id : {"clean", "power", "de-ess", "space", "echo", "tone", "output"})
        controls[static_cast<size_t>(index++)] = state.getRawParameterValue(id);
}
bool VocalLabProcessor::isBusesLayoutSupported(const BusesLayout& buses) const {
    const auto in = buses.getMainInputChannelSet(), out = buses.getMainOutputChannelSet();
    return (in == juce::AudioChannelSet::mono() && (out == in || out == juce::AudioChannelSet::stereo()))
        || (in == juce::AudioChannelSet::stereo() && out == in);
}
void VocalLabProcessor::prepareToPlay(double rate, int) {
    chain.prepare(rate);
    setLatencySamples(0);
}
void VocalLabProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals guard;
    if (getTotalNumInputChannels() == 1 && buffer.getNumChannels() == 2)
        buffer.copyFrom(1, 0, buffer, 0, 0, buffer.getNumSamples());
    d2nz::Settings values;
    values.clean = controls[0]->load(); values.power = controls[1]->load();
    values.deess = controls[2]->load(); values.space = controls[3]->load();
    values.echo = controls[4]->load(); values.tone = controls[5]->load();
    values.outputDb = controls[6]->load();
    chain.setSettings(values);
    const auto meters = chain.process(buffer.getArrayOfWritePointers(), buffer.getNumChannels(), buffer.getNumSamples());
    inputPeak.store(meters.inputPeak); outputPeak.store(meters.outputPeak); gainReduction.store(meters.reductionDb);
}
juce::AudioProcessorEditor* VocalLabProcessor::createEditor() {
    // Functional JUCE parameter editor for the DSP milestone; branded UI follows.
    return new juce::GenericAudioProcessorEditor(*this);
}
const juce::String VocalLabProcessor::getProgramName(int index) {
    static constexpr const char* names[] = {"Neutral", "Street Clean - prototype", "Velvet - prototype"};
    return names[juce::jlimit(0, 2, index)];
}
void VocalLabProcessor::setCurrentProgram(int index) {
    if (index < 0 || index >= getNumPrograms()) return;
    static constexpr float presets[3][7] = {
        {0,0,0,0,0,0,0}, {0.65f,0.4f,0.5f,0.05f,0,0.1f,0}, {0.4f,0.3f,0.4f,0.3f,0.15f,0.4f,0}
    };
    int i = 0;
    for (const auto* id : {"clean", "power", "de-ess", "space", "echo", "tone", "output"}) {
        auto* parameter = state.getParameter(id);
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(presets[index][i++]));
        parameter->endChangeGesture();
    }
    program = index;
}
void VocalLabProcessor::getStateInformation(juce::MemoryBlock& data) {
    auto tree = state.copyState();
    tree.setProperty("schemaVersion", 1, nullptr);
    tree.setProperty("program", program, nullptr);
    if (const auto xml = tree.createXml()) copyXmlToBinary(*xml, data);
}
void VocalLabProcessor::setStateInformation(const void* data, int size) {
    if (const auto xml = getXmlFromBinary(data, size)) {
        const auto tree = juce::ValueTree::fromXml(*xml);
        if (tree.hasType("D2NZState") && static_cast<int>(tree.getProperty("schemaVersion", 0)) == 1) {
            state.replaceState(tree);
            program = juce::jlimit(0, 2, static_cast<int>(tree.getProperty("program", 0)));
        }
    }
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new VocalLabProcessor; }
