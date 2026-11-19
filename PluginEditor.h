#pragma once

#include "PluginProcessor.h"

class CrtLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CrtLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;
};

struct KnobControl
{
    juce::Slider slider;
    juce::Label  label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

class LoFiErV2AudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit LoFiErV2AudioProcessorEditor(LoFiErV2AudioProcessor&);
    ~LoFiErV2AudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void initKnob(KnobControl& k, const juce::String& paramID, const juce::String& name,
                  const juce::String& suffix = "", juce::Colour accent = juce::Colour(0xFF39FF88));

    LoFiErV2AudioProcessor& audioProcessor;
    CrtLookAndFeel lnf;

    // 21 Knobs
    KnobControl kSubFc, kSubWarm, kSubIron, kSubThump;
    KnobControl kWow, kFlutter, kDropouts;
    KnobControl kGapDrive, kAzimuth;
    KnobControl kCompRate, kCompBits, kCompEmp, kCompAmt;
    KnobControl kHalo, kVinyl;
    KnobControl kStarve, kDrag;
    KnobControl kCemFc, kCemRes, kWoodBox, kMix;

    // Fault Mode Selector
    juce::ComboBox faultBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> faultAttachment;

    // Module bounds for painting cards
    juce::Rectangle<int> cardBounds[8];
    float uiFreeze = 0.0f;
    float uiSag    = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoFiErV2AudioProcessorEditor)
};
