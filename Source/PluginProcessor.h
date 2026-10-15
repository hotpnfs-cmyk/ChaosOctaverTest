#pragma once

#include <JuceHeader.h>
#include <array>

class OctaveChaosAudioProcessor : public juce::AudioProcessor
{
public:
    OctaveChaosAudioProcessor();
    ~OctaveChaosAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(
        juce::AudioBuffer<float>&,
        juce::MidiBuffer&
    ) override;

    juce::AudioProcessorEditor* createEditor() override;

    bool hasEditor() const override
    {
        return true;
    }

    const juce::String getName() const override
    {
        return JucePlugin_Name;
    }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }

    double getTailLengthSeconds() const override
    {
        return 0.2;
    }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }

    void setCurrentProgram(int) override {}

    const juce::String getProgramName(int) override
    {
        return {};
    }

    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;

    void setStateInformation(const void*, int) override;

    using APVTS = juce::AudioProcessorValueTreeState;

    APVTS parameters;

private:
    // Pitch shifters
    juce::dsp::PitchShifter<float> shifter1;
    juce::dsp::PitchShifter<float> shifter2;

    // Parameter layout
    static APVTS::ParameterLayout createParameterLayout();

    // Audio buffers
    juce::AudioBuffer<float> dryBuffer;

    // Processing state
    double currentSampleRate = 44100.0;

    juce::Random random;

    float chaosCurrent = 0.0f;
    float chaosTarget = 0.0f;

    int samplesUntilNewTarget = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        OctaveChaosAudioProcessor
    )
};
