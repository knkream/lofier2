#pragma once

#include <vector>
#include "PluginProcessor.h"

class ProConsoleLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ProConsoleLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;

    void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawComboBox(juce::Graphics&, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox&) override;
};

// Native JUCE port of lofi_visualizer.js (P31 CRT Phosphor Oscilloscope & Tape Reels)
class CrtVisualizerComponent : public juce::Component
{
public:
    explicit CrtVisualizerComponent(LoFiErV3AudioProcessor& p);
    void tick();
    void paint(juce::Graphics& g) override;

private:
    struct Spark { float x, y, life, sz; };

    void drawReel(juce::Graphics& g, float x, float y, float r, float angle, float pulse);

    LoFiErV3AudioProcessor& processor;
    float waveBuf[LoFiErV3AudioProcessor::kScopeSize] {};
    float envVal = 0.0f, transVal = 0.0f, dropFlag = 0.0f, sagVal = 0.0f;
    float bitsVal = 16.0f, srVal = 44100.0f;
    float reelAngle = 0.0f;
    std::vector<Spark> sparks;
    juce::Random rng;
};

struct KnobControl
{
    juce::Slider slider;
    juce::Label  label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

struct BypassLed
{
    juce::ToggleButton button;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
};

class LoFiErV3AudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit LoFiErV3AudioProcessorEditor(LoFiErV3AudioProcessor&);
    ~LoFiErV3AudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void initKnob(KnobControl& k, const juce::String& paramID, const juce::String& name,
                  const juce::String& suffix = "", juce::Colour accent = juce::Colour(0xFF39FF88),
                  bool compact = false);
    void initBypass(BypassLed& b, const juce::String& paramID);
    void drawScrew(juce::Graphics& g, float cx, float cy) const;
    void drawStereoMeter(juce::Graphics& g, juce::Rectangle<int> r, float lvlL, float lvlR, const juce::String& lbl) const;

    LoFiErV3AudioProcessor& audioProcessor;
    ProConsoleLookAndFeel lnf;
    CrtVisualizerComponent crtVisualizer;

    // Header Controls
    juce::ComboBox   presetBox;
    juce::TextButton prevPresetBtn { "<" }, nextPresetBtn { ">" };
    KnobControl      kInGain, kOutGain, kMix;

    // Module Knobs
    KnobControl kSubFc, kSubWarm, kSubIron, kSubThump;
    KnobControl kWow, kFlutter, kDropouts;
    KnobControl kGapDrive, kAzimuth;
    KnobControl kCompRate, kCompBits, kCompEmp, kCompAmt;
    KnobControl kHalo, kVinyl;
    KnobControl kStarve, kDrag;
    KnobControl kCemFc, kCemRes, kWoodBox;
    KnobControl kTapeDrive, kTapeBias;

    // Mode Selectors
    juce::ComboBox faultBox, tapeModeBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> faultAttachment, tapeModeAttachment;

    // Module Power Bypass LEDs
    BypassLed bSub, bTrans, bHead, bComp, bDust, bStarve, bCem;

    // Layout Regions
    juce::Rectangle<int> cardBounds[9];
    juce::Rectangle<int> inMeterBounds, outMeterBounds;

    float uiInL = 0.f, uiInR = 0.f, uiOutL = 0.f, uiOutR = 0.f;
    float uiDrop = 0.f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoFiErV3AudioProcessorEditor)
};
