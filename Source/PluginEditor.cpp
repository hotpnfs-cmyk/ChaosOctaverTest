#include "PluginEditor.h"

OctaveChaosAudioProcessorEditor::OctaveChaosAudioProcessorEditor (OctaveChaosAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (520, 300);
    title.setText ("OCTAVE CHAOS", juce::dontSendNotification);
    title.setJustificationType (juce::Justification::centred);
    title.setFont (juce::Font (25.0f, juce::Font::bold));
    addAndMakeVisible (title);

    for (auto* b : { &octave1, &octave2, &panic })
    {
        b->setClickingTogglesState (true);
        addAndMakeVisible (*b);
    }

    auto setupSlider = [this](juce::Slider& s, juce::Label& l, const juce::String& text)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);
        l.setText (text, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (s);
        addAndMakeVisible (l);
    };
    setupSlider (blend, blendLabel, "BLEND");
    setupSlider (chaos, chaosLabel, "CHAOS");
    setupSlider (speed, speedLabel, "SPEED");
    setupSlider (rise, riseLabel, "RISE");

    auto& apvts = audioProcessor.parameters;
    octave1Attachment = std::make_unique<ButtonAttachment>(apvts, "octave1", octave1);
    octave2Attachment = std::make_unique<ButtonAttachment>(apvts, "octave2", octave2);
    panicAttachment  = std::make_unique<ButtonAttachment>(apvts, "panic", panic);
    blendAttachment = std::make_unique<SliderAttachment>(apvts, "blend", blend);
    chaosAttachment = std::make_unique<SliderAttachment>(apvts, "chaos", chaos);
    speedAttachment = std::make_unique<SliderAttachment>(apvts, "speed", speed);
    riseAttachment = std::make_unique<SliderAttachment>(apvts, "rise", rise);
}

void OctaveChaosAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff111216));
    g.setColour (juce::Colour (0xffa6ff00));
    g.fillRoundedRectangle (18.0f, 14.0f, (float)getWidth() - 36.0f, 3.0f, 1.5f);
    g.setColour (juce::Colour (0xff25272e));
    g.drawRoundedRectangle (8.0f, 8.0f, (float)getWidth() - 16.0f, (float)getHeight() - 16.0f, 10.0f, 1.5f);
    g.setColour (juce::Colour (0xff777b84));
    g.setFont (12.0f);
    g.drawFittedText ("PITCH / DISSONANCE EFFECT  •  STUDIO ONE", 10, getHeight() - 26, getWidth() - 20, 16,
                      juce::Justification::centred, 1);
}

void OctaveChaosAudioProcessorEditor::resized()
{
    title.setBounds (20, 24, getWidth() - 40, 38);
    octave1.setBounds (40, 76, 120, 34);
    octave2.setBounds (200, 76, 120, 34);
    panic.setBounds (360, 76, 120, 34);

    const int y = 132, w = 100, gap = 22, start = 38;
    juce::Slider* sliders[] = { &blend, &chaos, &speed, &rise };
    juce::Label* labels[] = { &blendLabel, &chaosLabel, &speedLabel, &riseLabel };
    for (int i = 0; i < 4; ++i)
    {
        const int x = start + i * (w + gap);
        sliders[i]->setBounds (x, y, w, 112);
        labels[i]->setBounds (x, y + 105, w, 20);
    }
}
