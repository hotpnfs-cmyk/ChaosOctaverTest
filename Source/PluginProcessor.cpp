#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr auto paramOct1 = "octave1";
    constexpr auto paramOct2 = "octave2";
    constexpr auto paramBlend = "blend";
    constexpr auto paramPanic = "panic";
    constexpr auto paramChaos = "chaos";
    constexpr auto paramSpeed = "speed";
    constexpr auto paramRise = "rise";
}

OctaveChaosAudioProcessor::OctaveChaosAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

OctaveChaosAudioProcessor::APVTS::ParameterLayout OctaveChaosAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;
    layout.add (std::make_unique<juce::AudioParameterBool>(paramOct1, "Octave +1", false));
    layout.add (std::make_unique<juce::AudioParameterBool>(paramOct2, "Octave +2", false));
    layout.add (std::make_unique<juce::AudioParameterFloat>(paramBlend, "Blend",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.65f));
    layout.add (std::make_unique<juce::AudioParameterBool>(paramPanic, "Panic", false));
    layout.add (std::make_unique<juce::AudioParameterFloat>(paramChaos, "Chaos",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.25f));
    layout.add (std::make_unique<juce::AudioParameterFloat>(paramSpeed, "Speed",
        juce::NormalisableRange<float>(0.1f, 20.0f, 0.01f, 0.5f), 5.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat>(paramRise, "Rise",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.001f), 0.15f));
    return layout;
}

void OctaveChaosAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock,
                                 (juce::uint32) getTotalNumOutputChannels() };
    shifter1.reset();
    shifter2.reset();
    shifter1.prepare(spec);
    shifter2.prepare(spec);
    shifter1.setPitchRatio(2.0f);
    shifter2.setPitchRatio(4.0f);
    dryBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    chaosCurrent = chaosTarget = 0.0f;
    samplesUntilNewTarget = 0;
}

void OctaveChaosAudioProcessor::releaseResources()
{
}

bool OctaveChaosAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return in == out && (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo());
}

void OctaveChaosAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = buffer.getNumChannels();
    const int n = buffer.getNumSamples();
    if (dryBuffer.getNumSamples() < n || dryBuffer.getNumChannels() != channels)
        dryBuffer.setSize(channels, n, false, false, true);
    dryBuffer.makeCopyOf(buffer, true);

    const bool oct1 = parameters.getRawParameterValue(paramOct1)->load() > 0.5f;
    const bool oct2 = parameters.getRawParameterValue(paramOct2)->load() > 0.5f;
    const bool panic = parameters.getRawParameterValue(paramPanic)->load() > 0.5f;
    const float blend = parameters.getRawParameterValue(paramBlend)->load();
    const float chaos = parameters.getRawParameterValue(paramChaos)->load();
    const float speed = parameters.getRawParameterValue(paramSpeed)->load();
    const float rise = parameters.getRawParameterValue(paramRise)->load();

    // Reinitialize the pitch shifters from the dry signal for independent octave voices.
    juce::AudioBuffer<float> voice1(channels, n), voice2(channels, n);
    voice1.makeCopyOf(dryBuffer, true);
    voice2.makeCopyOf(dryBuffer, true);

    juce::dsp::AudioBlock<float> block1(voice1);
    juce::dsp::AudioBlock<float> block2(voice2);
    auto ctx1 = juce::dsp::ProcessContextReplacing<float>(block1);
    auto ctx2 = juce::dsp::ProcessContextReplacing<float>(block2);

    const float riseSeconds = juce::jmax(0.001f, rise);
    const float smooth = 1.0f - std::exp(-1.0f / (float)(currentSampleRate * riseSeconds));
    const int targetPeriod = juce::jmax(1, (int)(currentSampleRate / speed));

    for (int i = 0; i < n; ++i)
    {
        if (samplesUntilNewTarget <= 0)
        {
            chaosTarget = panic ? (random.nextFloat() * 2.0f - 1.0f) * chaos : 0.0f;
            samplesUntilNewTarget = targetPeriod;
        }
        --samplesUntilNewTarget;
        chaosCurrent += (chaosTarget - chaosCurrent) * (rise <= 0.001f ? 1.0f : smooth);
    }

    // PitchShifter is block-based; the smoothed chaos value gently modulates the octave ratios.
    const float semitones = panic ? chaosCurrent * 2.0f : 0.0f;
    shifter1.setPitchRatio(juce::jlimit(1.5f, 2.5f, std::pow(2.0f, (12.0f + semitones) / 12.0f)));
    shifter2.setPitchRatio(juce::jlimit(3.0f, 5.0f, std::pow(2.0f, (24.0f + semitones) / 12.0f)));
    shifter1.process(ctx1);
    shifter2.process(ctx2);

    for (int ch = 0; ch < channels; ++ch)
    {
        auto* out = buffer.getWritePointer(ch);
        const auto* dry = dryBuffer.getReadPointer(ch);
        const auto* up1 = voice1.getReadPointer(ch);
        const auto* up2 = voice2.getReadPointer(ch);

        for (int i = 0; i < n; ++i)
        {
            float wet = 0.0f;
            int active = 0;
            if (oct1) { wet += up1[i]; ++active; }
            if (oct2) { wet += up2[i]; ++active; }
            if (active > 1) wet /= (float) active;
            if (active == 0) wet = dry[i];
            out[i] = (active == 0) ? dry[i] : dry[i] * (1.0f - blend) + wet * blend;
        }
    }
}

juce::AudioProcessorEditor* OctaveChaosAudioProcessor::createEditor()
{
    return new OctaveChaosAudioProcessorEditor (*this);
}

void OctaveChaosAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void OctaveChaosAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (parameters.state.getType()))
        parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OctaveChaosAudioProcessor();
}
