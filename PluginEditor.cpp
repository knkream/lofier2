#include "PluginEditor.h"

namespace
{
    const juce::Colour kBgDark      { 0xFF101214 };
    const juce::Colour kCardBg      { 0xFF171A1E };
    const juce::Colour kCardBorder  { 0xFF282E35 };
    const juce::Colour kPhosphor    { 0xFF39FF88 };
    const juce::Colour kAmber       { 0xFFFFB03A };
    const juce::Colour kCoral       { 0xFFFF5C5C };
    const juce::Colour kTextMuted   { 0xFF8B949E };
}

CrtLookAndFeel::CrtLookAndFeel()
{
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF0E1012));
    setColour(juce::ComboBox::outlineColourId,    kCardBorder);
    setColour(juce::ComboBox::textColourId,       kPhosphor);
    setColour(juce::ComboBox::arrowColourId,      kPhosphor);
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xFF14171A));
    setColour(juce::PopupMenu::textColourId,       juce::Colour(0xFFE6EDF3));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xFF233A2C));
}

void CrtLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                      float sliderPos, float startAngle, float endAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                               static_cast<float>(width), static_cast<float>(height)).reduced(6.0f);
    const float radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float angle = startAngle + sliderPos * (endAngle - startAngle);
    const auto accent = slider.findColour(juce::Slider::rotarySliderFillColourId);

    // Background track
    juce::Path bgArc;
    bgArc.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, startAngle, endAngle, true);
    g.setColour(juce::Colour(0xFF262C33));
    g.strokePath(bgArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Active value arc
    juce::Path valArc;
    valArc.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, startAngle, angle, true);
    g.setColour(accent);
    g.strokePath(valArc, juce::PathStrokeType(3.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Inner hardware cap
    const float capR = radius * 0.72f;
    g.setColour(juce::Colour(0xFF1F242A));
    g.fillEllipse(centre.x - capR, centre.y - capR, capR * 2.0f, capR * 2.0f);
    g.setColour(juce::Colour(0xFF323942));
    g.drawEllipse(centre.x - capR, centre.y - capR, capR * 2.0f, capR * 2.0f, 1.0f);

    // Pointer tick
    juce::Path pointer;
    pointer.addRoundedRectangle(-1.4f, -capR + 3.0f, 2.8f, capR * 0.52f, 1.0f);
    g.setColour(juce::Colours::white);
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
}

LoFiErV2AudioProcessorEditor::LoFiErV2AudioProcessorEditor(LoFiErV2AudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&lnf);

    initKnob(kSubFc,    "subFc",    "X-OVER",  " Hz", kPhosphor);
    initKnob(kSubWarm,  "subWarm",  "WARMTH",  "",    kPhosphor);
    initKnob(kSubIron,  "subIron",  "IRON",    "",    kPhosphor);
    initKnob(kSubThump, "subThump", "THUMP",   "",    kPhosphor);

    initKnob(kWow,      "wow",      "WOW",     "",    kAmber);
    initKnob(kFlutter,  "flutter",  "FLUTTER", "",    kAmber);
    initKnob(kDropouts, "dropouts", "DROPOUT", "",    kAmber);

    initKnob(kGapDrive, "gapDrive", "DRIVE",   "",    kAmber);
    initKnob(kAzimuth,  "azimuth",  "AZIMUTH", "",    kAmber);

    initKnob(kCompRate, "compRate", "CLOCK",   " Hz", kPhosphor);
    initKnob(kCompBits, "compBits", "BITS",    "",    kPhosphor);
    initKnob(kCompEmp,  "compEmp",  "PRE-EMP", "",    kPhosphor);
    initKnob(kCompAmt,  "compAmt",  "COMP",    "",    kPhosphor);

    faultBox.addItemList({"OFF", "DAC DRIFT", "GSM 1-BAR", "RAM ROT"}, 1);
    addAndMakeVisible(faultBox);
    faultAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "faultMode", faultBox);

    initKnob(kHalo,     "halo",     "HALO",    "",    kPhosphor);
    initKnob(kVinyl,    "vinyl",    "VINYL",   "",    kPhosphor);

    initKnob(kStarve,   "starve",   "STARVE",  "",    kCoral);
    initKnob(kDrag,     "drag",     "DRAG",    "",    kCoral);

    initKnob(kCemFc,    "cemFc",    "CUTOFF",  " Hz", kPhosphor);
    initKnob(kCemRes,   "cemRes",   "RES",     "",    kPhosphor);
    initKnob(kWoodBox,  "woodBox",  "WOOD",    "",    kAmber);
    initKnob(kMix,      "mix",      "MIX",     "",    juce::Colours::white);

    setSize(900, 470);
    startTimerHz(30);
}

LoFiErV2AudioProcessorEditor::~LoFiErV2AudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void LoFiErV2AudioProcessorEditor::initKnob(KnobControl& k, const juce::String& paramID,
                                            const juce::String& name, const juce::String& suffix, juce::Colour accent)
{
    k.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 62, 16);
    k.slider.setTextValueSuffix(suffix);
    k.slider.setColour(juce::Slider::rotarySliderFillColourId, accent);
    k.slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFFD0D7DE));
    k.slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(k.slider);

    k.label.setText(name, juce::dontSendNotification);
    k.label.setJustificationType(juce::Justification::centred);
    k.label.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    k.label.setColour(juce::Label::textColourId, kTextMuted);
    addAndMakeVisible(k.label);

    k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, paramID, k.slider);
}

void LoFiErV2AudioProcessorEditor::timerCallback()
{
    const float newFrz = audioProcessor.telemetryFreeze.load();
    const float newSag = audioProcessor.telemetrySag.load();
    uiFreeze = uiFreeze * 0.7f + newFrz * 0.3f;
    uiSag    = uiSag * 0.8f + newSag * 0.2f;
    repaint();
}

void LoFiErV2AudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(kBgDark);

    // Top Header Bar
    auto header = getLocalBounds().removeFromTop(38);
    g.setColour(juce::Colour(0xFF14171B));
    g.fillRect(header);
    g.setColour(kCardBorder);
    g.drawHorizontalLine(header.getBottom() - 1, 0.0f, static_cast<float>(getWidth()));

    g.setColour(kPhosphor);
    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    g.drawText("LO-FI-ER // V2 ANALOG & FAULT CONSOLE", header.reduced(16, 0), juce::Justification::centredLeft);

    // Live Telemetry LEDs in Header
    auto drawLed = [&](int x, const juce::String& text, float intensity, juce::Colour col)
    {
        g.setColour(col.withAlpha(0.15f + std::clamp(intensity, 0.0f, 1.0f) * 0.85f));
        g.fillEllipse(static_cast<float>(x), 14.0f, 10.0f, 10.0f);
        g.setColour(kTextMuted);
        g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
        g.drawText(text, x + 15, 10, 65, 18, juce::Justification::centredLeft);
    };
    drawLed(getWidth() - 185, "VOLT SAG", uiSag, kCoral);
    drawLed(getWidth() - 95,  "FREEZE",   uiFreeze, kAmber);

    // Module Cards
    static const char* titles[8] = {
        "01 // SUB ANCHOR", "02 // TRANSPORT", "03 // TAPE HEAD", "04 // COMPANDER",
        "05 // FAULT ENGINE", "06 // LIVING DUST", "07 // VOLTAGE STARVE", "08 // CEM3320 & OUT"
    };

    for (int i = 0; i < 8; ++i)
    {
        auto r = cardBounds[i].toFloat();
        g.setColour(kCardBg);
        g.fillRoundedRectangle(r, 6.0f);
        g.setColour(kCardBorder);
        g.drawRoundedRectangle(r, 6.0f, 1.0f);

        auto titleArea = cardBounds[i].withHeight(22).reduced(8, 2);
        g.setColour(kPhosphor.withAlpha(0.85f));
        g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
        g.drawText(titles[i], titleArea, juce::Justification::centredLeft);
    }
}

void LoFiErV2AudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(38);
    area.reduce(12, 12);

    const int gap = 10;
    const int rowH = (area.getHeight() - gap) / 2;
    auto topRow = area.removeFromTop(rowH);
    area.removeFromTop(gap);
    auto botRow = area;

    auto splitRow = [&](juce::Rectangle<int> row, std::initializer_list<int> weights, int startIdx)
    {
        int totalW = 0;
        for (int w : weights) totalW += w;
        const int avail = row.getWidth() - gap * (static_cast<int>(weights.size()) - 1);
        int idx = startIdx;
        for (int w : weights)
        {
            int px = (avail * w) / totalW;
            cardBounds[idx++] = row.removeFromLeft(px);
            row.removeFromLeft(gap);
        }
    };

    splitRow(topRow, { 4, 3, 2, 4 }, 0);
    splitRow(botRow, { 2, 2, 2, 4 }, 4);

    auto layoutKnobs = [](juce::Rectangle<int> card, std::initializer_list<KnobControl*> knobs)
    {
        card.removeFromTop(26);
        card.reduce(6, 6);
        const int colW = card.getWidth() / static_cast<int>(knobs.size());
        for (auto* k : knobs)
        {
            auto col = card.removeFromLeft(colW);
            k->label.setBounds(col.removeFromTop(16));
            k->slider.setBounds(col.reduced(2, 0));
        }
    };

    layoutKnobs(cardBounds[0], { &kSubFc, &kSubWarm, &kSubIron, &kSubThump });
    layoutKnobs(cardBounds[1], { &kWow, &kFlutter, &kDropouts });
    layoutKnobs(cardBounds[2], { &kGapDrive, &kAzimuth });
    layoutKnobs(cardBounds[3], { &kCompRate, &kCompBits, &kCompEmp, &kCompAmt });

    // Card 4: Fault Engine ComboBox
    auto faultArea = cardBounds[4];
    faultArea.removeFromTop(28);
    faultBox.setBounds(faultArea.reduced(14, 48));

    layoutKnobs(cardBounds[5], { &kHalo, &kVinyl });
    layoutKnobs(cardBounds[6], { &kStarve, &kDrag });
    layoutKnobs(cardBounds[7], { &kCemFc, &kCemRes, &kWoodBox, &kMix });
}
