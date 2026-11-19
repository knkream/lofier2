#include "PluginEditor.h"

namespace
{
    const juce::Colour kChassisTop  { 0xFF171A1F };
    const juce::Colour kChassisBot  { 0xFF0C0E11 };
    const juce::Colour kCardBg      { 0xFF14171C };
    const juce::Colour kCardHeader  { 0xFF1B2027 };
    const juce::Colour kCardBorder  { 0xFF282F38 };
    const juce::Colour kPhosphor    { 0xFF39FF88 };
    const juce::Colour kAmber       { 0xFFFFB03A };
    const juce::Colour kCoral       { 0xFFFF5544 };
    const juce::Colour kCyan        { 0xFF4CC9F0 };
    const juce::Colour kTextBright  { 0xFFE6EDF3 };
    const juce::Colour kTextMuted   { 0xFF7D8590 };
}

//==============================================================================
ProConsoleLookAndFeel::ProConsoleLookAndFeel()
{
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF0C0F12));
    setColour(juce::ComboBox::outlineColourId,    kCardBorder);
    setColour(juce::ComboBox::textColourId,       kPhosphor);
    setColour(juce::ComboBox::arrowColourId,      kPhosphor);

    setColour(juce::PopupMenu::backgroundColourId,            juce::Colour(0xFF12161B));
    setColour(juce::PopupMenu::textColourId,                  kTextBright);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xFF1E3529));
    setColour(juce::PopupMenu::highlightedTextColourId,       kPhosphor);

    setColour(juce::TextButton::buttonColourId,   juce::Colour(0xFF1A1F26));
    setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFF26303B));
    setColour(juce::TextButton::textColourOffId,  kTextBright);
}

void ProConsoleLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPos, float startAngle, float endAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                               static_cast<float>(width), static_cast<float>(height)).reduced(4.0f);
    const float radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float angle = startAngle + sliderPos * (endAngle - startAngle);
    const auto accent = slider.findColour(juce::Slider::rotarySliderFillColourId);

    // Subtle scale ticks around outer perimeter
    const int numTicks = 11;
    g.setColour(juce::Colour(0xFF333B46));
    for (int i = 0; i < numTicks; ++i)
    {
        const float t = static_cast<float>(i) / static_cast<float>(numTicks - 1);
        const float a = startAngle + t * (endAngle - startAngle) - juce::MathConstants<float>::halfPi;
        const float r1 = radius + 0.5f;
        const float r2 = radius + 3.0f;
        g.drawLine(centre.x + std::cos(a) * r1, centre.y + std::sin(a) * r1,
                   centre.x + std::cos(a) * r2, centre.y + std::sin(a) * r2, 1.0f);
    }

    // Track arc
    const float arcR = radius - 2.0f;
    juce::Path bgArc;
    bgArc.addCentredArc(centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour(juce::Colour(0xFF222830));
    g.strokePath(bgArc, juce::PathStrokeType(2.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Active phosphor glow + crisp value arc
    juce::Path valArc;
    valArc.addCentredArc(centre.x, centre.y, arcR, arcR, 0.0f, startAngle, angle, true);
    g.setColour(accent.withAlpha(0.22f));
    g.strokePath(valArc, juce::PathStrokeType(5.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(accent);
    g.strokePath(valArc, juce::PathStrokeType(2.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Drop shadow under aluminum cap
    const float capR = arcR * 0.74f;
    g.setColour(juce::Colours::black.withAlpha(0.55f));
    g.fillEllipse(centre.x - capR + 1.0f, centre.y - capR + 2.2f, capR * 2.0f, capR * 2.0f);

    // Machined metallic cap gradient
    juce::ColourGradient capGrad(juce::Colour(0xFF2E353F), centre.x, centre.y - capR,
                                 juce::Colour(0xFF15191E), centre.x, centre.y + capR, false);
    g.setGradientFill(capGrad);
    g.fillEllipse(centre.x - capR, centre.y - capR, capR * 2.0f, capR * 2.0f);

    // Bezel rim highlight
    g.setColour(juce::Colour(0xFF414A56));
    g.drawEllipse(centre.x - capR, centre.y - capR, capR * 2.0f, capR * 2.0f, 1.0f);

    // Pointer indicator
    juce::Path pointer;
    pointer.addRoundedRectangle(-1.3f, -capR + 2.5f, 2.6f, capR * 0.54f, 1.0f);
    g.setColour(juce::Colours::white);
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
}

void ProConsoleLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& btn, bool, bool)
{
    const auto r = btn.getLocalBounds().toFloat().reduced(3.0f);
    const float d = std::min(r.getWidth(), r.getHeight());
    const auto led = juce::Rectangle<float>(d, d).withCentre(r.getCentre());
    const bool on = btn.getToggleState();

    // Recessed socket
    g.setColour(juce::Colour(0xFF090B0D));
    g.fillEllipse(led.expanded(1.5f));

    if (on)
    {
        g.setColour(kPhosphor.withAlpha(0.32f));
        g.fillEllipse(led.expanded(2.5f));
        g.setColour(kPhosphor);
        g.fillEllipse(led);
        g.setColour(juce::Colours::white.withAlpha(0.75f));
        g.fillEllipse(led.reduced(d * 0.30f).translated(-0.7f, -0.7f));
    }
    else
    {
        g.setColour(juce::Colour(0xFF2A3139));
        g.fillEllipse(led);
    }
}

void ProConsoleLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool,
                                         int, int, int, int, juce::ComboBox& box)
{
    auto r = juce::Rectangle<float>(0.5f, 0.5f, static_cast<float>(width) - 1.0f, static_cast<float>(height) - 1.0f);
    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(r, 4.0f);
    g.setColour(box.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(r, 4.0f, 1.0f);

    // Small chevron arrow
    juce::Path arrow;
    const float ax = static_cast<float>(width) - 14.0f;
    const float ay = static_cast<float>(height) * 0.5f;
    arrow.startNewSubPath(ax - 3.5f, ay - 2.0f);
    arrow.lineTo(ax, ay + 2.5f);
    arrow.lineTo(ax + 3.5f, ay - 2.0f);
    g.setColour(box.findColour(juce::ComboBox::arrowColourId));
    g.strokePath(arrow, juce::PathStrokeType(1.5f));
}

//==============================================================================
CrtVisualizerComponent::CrtVisualizerComponent(LoFiErV3AudioProcessor& p) : processor(p)
{
    std::fill(std::begin(waveBuf), std::end(waveBuf), 0.0f);
}

void CrtVisualizerComponent::tick()
{
    // Read lock-free ring buffer into ordered 128-sample display buffer
    const int wp = processor.scopeWritePos.load(std::memory_order_acquire);
    for (int i = 0; i < LoFiErV3AudioProcessor::kScopeSize; ++i)
    {
        const int idx = (wp + i) % LoFiErV3AudioProcessor::kScopeSize;
        waveBuf[i] = processor.scopeRing[static_cast<size_t>(idx)].load(std::memory_order_relaxed);
    }

    const float newEnv   = processor.telEnv.load(std::memory_order_relaxed);
    const float newTrans = processor.telTrans.load(std::memory_order_relaxed);
    const float newDrop  = processor.telDrop.load(std::memory_order_relaxed);
    const float newSag   = processor.telSag.load(std::memory_order_relaxed);

    envVal   = envVal   * 0.65f + newEnv   * 0.35f;
    transVal = transVal * 0.55f + newTrans * 0.45f;
    dropFlag = (newDrop > 0.5f) ? 1.0f : (dropFlag * 0.72f);
    sagVal   = sagVal   * 0.75f + newSag   * 0.25f;
    bitsVal  = processor.telBits.load(std::memory_order_relaxed);
    srVal    = processor.telSr.load(std::memory_order_relaxed);

    if (transVal > 0.32f && rng.nextFloat() < 0.38f && sparks.size() < 18)
    {
        sparks.push_back({
            0.12f + rng.nextFloat() * 0.76f,
            0.20f + rng.nextFloat() * 0.60f,
            1.0f,
            1.5f + rng.nextFloat() * 2.2f
        });
    }

    const float speed = std::max(0.008f, 0.065f * (1.0f - sagVal * 0.78f) - transVal * 0.018f);
    reelAngle += speed;

    for (int s = static_cast<int>(sparks.size()) - 1; s >= 0; --s)
    {
        sparks[static_cast<size_t>(s)].life -= 0.08f;
        if (sparks[static_cast<size_t>(s)].life <= 0.0f)
            sparks.erase(sparks.begin() + s);
    }

    repaint();
}

void CrtVisualizerComponent::drawReel(juce::Graphics& g, float x, float y, float r, float angle, float pulse)
{
    g.setColour(juce::Colour::fromFloatRGBA(0.16f, 0.52f, 0.32f, 0.42f + pulse * 0.28f));
    g.drawEllipse(x - r, y - r, r * 2.0f, r * 2.0f, 1.15f);
    const float hubR = r * 0.28f;
    g.drawEllipse(x - hubR, y - hubR, hubR * 2.0f, hubR * 2.0f, 1.15f);

    for (int k = 0; k < 3; ++k)
    {
        const float a = angle + static_cast<float>(k) * (juce::MathConstants<float>::twoPi / 3.0f);
        g.drawLine(x + std::cos(a) * (r * 0.30f), y + std::sin(a) * (r * 0.30f),
                   x + std::cos(a) * (r * 0.92f), y + std::sin(a) * (r * 0.92f), 1.15f);
    }
}

void CrtVisualizerComponent::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const float w = bounds.getWidth();
    const float h = bounds.getHeight();

    // 1. Deep P31 CRT Phosphor Glass Void (with subtle radial tube vignette)
    juce::ColourGradient crtBg(juce::Colour::fromFloatRGBA(0.032f, 0.055f, 0.042f, 1.0f), w * 0.5f, h * 0.5f,
                               juce::Colour::fromFloatRGBA(0.014f, 0.024f, 0.018f, 1.0f), 0.0f, 0.0f, true);
    g.setGradientFill(crtBg);
    g.fillRoundedRectangle(bounds, 6.0f);

    // Clip inner content to CRT glass
    g.saveState();
    juce::Path clip;
    clip.addRoundedRectangle(bounds.reduced(1.5f), 5.0f);
    g.reduceClipRegion(clip);

    // 2. Oscilloscope Graticule Grid (8x4)
    g.setColour(juce::Colour::fromFloatRGBA(0.09f, 0.22f, 0.14f, 0.72f));
    for (int gx = 1; gx < 8; ++gx)
    {
        const float x = (w / 8.0f) * static_cast<float>(gx);
        g.drawLine(x, 4.0f, x, h - 4.0f, 0.9f);
    }
    for (int gy = 1; gy < 4; ++gy)
    {
        const float y = (h / 4.0f) * static_cast<float>(gy);
        g.drawLine(4.0f, y, w - 4.0f, y, 0.9f);
    }

    // 3. Wireframe Tape Reels & Sagging Tension Arm
    const float reelR = std::min(w, h) * 0.28f;
    drawReel(g, w * 0.20f, h * 0.52f, reelR, reelAngle, envVal);
    drawReel(g, w * 0.80f, h * 0.52f, reelR, reelAngle * 0.96f + 0.4f, envVal);

    const float tensionY = h * 0.82f + (sagVal * 12.0f) - (transVal * 4.5f);
    g.setColour(juce::Colour::fromFloatRGBA(0.18f, 0.58f, 0.36f, 0.55f));
    juce::Path arm;
    arm.startNewSubPath(w * 0.20f, h * 0.78f);
    arm.lineTo(w * 0.50f, tensionY);
    arm.lineTo(w * 0.80f, h * 0.78f);
    g.strokePath(arm, juce::PathStrokeType(1.25f));

    // 4. B-H Magnetic Hysteresis Ring
    const float cx = w * 0.5f, cy = h * 0.52f;
    const float baseR = std::min(w, h) * (0.20f + envVal * 0.13f);
    g.setColour(juce::Colour::fromFloatRGBA(0.22f, 1.0f, 0.53f, 0.20f + envVal * 0.45f));
    g.drawEllipse(cx - baseR, cy - baseR, baseR * 2.0f, baseR * 2.0f, 1.8f);

    // 5. Dual-Pass P31 Phosphor Waveform Trace
    const float levels = std::pow(2.0f, std::max(3.0f, bitsVal - 1.0f));
    const int srStep   = std::max(1, static_cast<int>(std::floor((44100.0f / std::max(1500.0f, srVal)) * 1.4f)));
    const float tear   = (dropFlag > 0.5f) ? ((rng.nextFloat() - 0.5f) * 20.0f) : 0.0f;

    juce::Path trace;
    float heldVal = 0.0f;
    for (int i = 0; i < LoFiErV3AudioProcessor::kScopeSize; ++i)
    {
        if (i % srStep == 0)
            heldVal = std::round(std::clamp(waveBuf[i], -1.0f, 1.0f) * levels) / levels;

        const float px = 10.0f + ((w - 20.0f) * (static_cast<float>(i) / 127.0f))
                       + ((i > 48 && i < 84) ? tear : 0.0f);
        const float py = (h * 0.52f) - (heldVal * (h * 0.36f));
        if (i == 0) trace.startNewSubPath(px, py);
        else        trace.lineTo(px, py);
    }

    if (dropFlag > 0.5f)
    {
        g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.28f, 0.20f, 0.30f));
        g.strokePath(trace, juce::PathStrokeType(4.8f));
        g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.40f, 0.30f, 0.98f));
        g.strokePath(trace, juce::PathStrokeType(1.6f));
    }
    else
    {
        g.setColour(juce::Colour::fromFloatRGBA(0.22f, 1.0f, 0.53f, 0.24f));
        g.strokePath(trace, juce::PathStrokeType(4.8f));
        g.setColour(juce::Colour::fromFloatRGBA(0.75f, 1.0f, 0.82f, 0.96f));
        g.strokePath(trace, juce::PathStrokeType(1.6f));
    }

    // 6. Oxide / Dust Micro-Sparks
    for (const auto& sp : sparks)
    {
        g.setColour(juce::Colour::fromFloatRGBA(0.80f, 1.0f, 0.65f, sp.life * 0.85f));
        g.fillRect(sp.x * w, sp.y * h, sp.sz, sp.sz);
    }

    // 7. Live Telemetry Corner Readouts
    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 10.5f, juce::Font::bold));

    g.setColour(juce::Colour::fromFloatRGBA(0.22f, 1.0f, 0.53f, 0.90f));
    const juce::String sysStr = "SYS: " + juce::String(bitsVal, 1) + "B // " + juce::String(srVal / 1000.0f, 1) + "KHZ";
    g.drawText(sysStr, 8, 5, 190, 14, juce::Justification::centredLeft);

    const float railVolts = std::max(1.8f, 9.0f - (sagVal * 5.2f) - (sagVal * envVal * 2.4f));
    g.setColour(railVolts < 5.5f ? juce::Colour::fromFloatRGBA(1.0f, 0.32f, 0.22f, 0.95f)
                                 : juce::Colour::fromFloatRGBA(0.22f, 1.0f, 0.53f, 0.90f));
    g.drawText("RAIL: " + juce::String(railVolts, 1) + "V", static_cast<int>(w) - 108, 5, 100, 14, juce::Justification::centredRight);

    if (dropFlag > 0.5f)
    {
        g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.30f, 0.22f, 0.98f));
        g.drawText("STATUS: PKT_LOSS // FRAME_HOLD", 8, static_cast<int>(h) - 18, 250, 14, juce::Justification::centredLeft);
    }
    else
    {
        g.setColour(juce::Colour::fromFloatRGBA(0.22f, 0.82f, 0.48f, 0.78f));
        g.drawText(envVal > 0.65f ? "FLUX: SATURATED" : "FLUX: NOMINAL", 8, static_cast<int>(h) - 18, 160, 14, juce::Justification::centredLeft);
    }

    // 8. Horizontal CRT Scanline Overlay & Top Glass Specular Glare
    g.setColour(juce::Colours::black.withAlpha(0.22f));
    for (float sy = 0.0f; sy < h; sy += 3.0f)
        g.drawHorizontalLine(static_cast<int>(sy), 0.0f, w);

    juce::ColourGradient glare(juce::Colours::white.withAlpha(0.055f), 0.0f, 0.0f,
                               juce::Colours::transparentWhite, 0.0f, h * 0.42f, false);
    g.setGradientFill(glare);
    g.fillRect(0.0f, 0.0f, w, h * 0.42f);

    g.restoreState();

    // Recessed hardware bezel
    g.setColour(juce::Colour(0xFF080A0C));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 2.0f);
    g.setColour(juce::Colour::fromFloatRGBA(0.18f, 0.42f, 0.28f, 0.85f));
    g.drawRoundedRectangle(bounds.reduced(1.5f), 5.0f, 1.0f);
}

//==============================================================================
LoFiErV3AudioProcessorEditor::LoFiErV3AudioProcessorEditor(LoFiErV3AudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), crtVisualizer(p)
{
    setLookAndFeel(&lnf);
    addAndMakeVisible(crtVisualizer);

    // Preset bar
    const auto& presets = LoFiErV3AudioProcessor::getPresets();
    for (int i = 0; i < static_cast<int>(presets.size()); ++i)
        presetBox.addItem(presets[static_cast<size_t>(i)].name, i + 1);
    presetBox.setSelectedId(audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this]
    {
        const int idx = presetBox.getSelectedId() - 1;
        if (idx >= 0) audioProcessor.applyPreset(idx);
    };
    addAndMakeVisible(presetBox);

    prevPresetBtn.onClick = [this]
    {
        const int n = audioProcessor.getNumPrograms();
        const int next = (audioProcessor.getCurrentProgram() - 1 + n) % n;
        audioProcessor.applyPreset(next);
        presetBox.setSelectedId(next + 1, juce::dontSendNotification);
    };
    nextPresetBtn.onClick = [this]
    {
        const int n = audioProcessor.getNumPrograms();
        const int next = (audioProcessor.getCurrentProgram() + 1) % n;
        audioProcessor.applyPreset(next);
        presetBox.setSelectedId(next + 1, juce::dontSendNotification);
    };
    addAndMakeVisible(prevPresetBtn);
    addAndMakeVisible(nextPresetBtn);

    // Master Header Knobs
    initKnob(kInGain,  "inGain",  "IN",  " dB", kCyan, true);
    initKnob(kOutGain, "outGain", "OUT", " dB", kCyan, true);
    initKnob(kMix,     "mix",     "MIX", "",    juce::Colours::white, true);

    // Module Knobs
    initKnob(kSubFc,     "subFc",     "X-OVER",  " Hz", kPhosphor);
    initKnob(kSubWarm,   "subWarm",   "WARMTH",  "",    kPhosphor);
    initKnob(kSubIron,   "subIron",   "IRON",    "",    kPhosphor);
    initKnob(kSubThump,  "subThump",  "THUMP",   "",    kPhosphor);

    initKnob(kWow,       "wow",       "WOW",     "",    kAmber);
    initKnob(kFlutter,   "flutter",   "FLUTTER", "",    kAmber);
    initKnob(kDropouts,  "dropouts",  "DROPOUT", "",    kAmber);

    initKnob(kGapDrive,  "gapDrive",  "DRIVE",   "",    kAmber);
    initKnob(kAzimuth,   "azimuth",   "AZIMUTH", "",    kAmber);

    initKnob(kCompRate,  "compRate",  "CLOCK",   " Hz", kCyan);
    initKnob(kCompBits,  "compBits",  "BITS",    "",    kCyan);
    initKnob(kCompEmp,   "compEmp",   "PRE-EMP", "",    kCyan);
    initKnob(kCompAmt,   "compAmt",   "COMP",    "",    kCyan);

    faultBox.addItemList({"OFF", "DAC DRIFT", "GSM 1-BAR", "RAM ROT"}, 1);
    addAndMakeVisible(faultBox);
    faultAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "faultMode", faultBox);

    initKnob(kHalo,      "halo",      "HALO",    "",    kPhosphor);
    initKnob(kVinyl,     "vinyl",     "VINYL",   "",    kPhosphor);

    initKnob(kStarve,    "starve",    "STARVE",  "",    kCoral);
    initKnob(kDrag,      "drag",      "DRAG",    "",    kCoral);

    initKnob(kCemFc,     "cemFc",     "CUTOFF",  " Hz", kPhosphor);
    initKnob(kCemRes,    "cemRes",    "RES",     "",    kPhosphor);
    initKnob(kWoodBox,   "woodBox",   "WOOD",    "",    kAmber);

    tapeModeBox.addItemList({"BYPASS", "STUDIO 15IPS", "PRO 7.5IPS", "CASSETTE", "HEAVY IRON", "AGED REEL"}, 1);
    addAndMakeVisible(tapeModeBox);
    tapeModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "tapeMode", tapeModeBox);
    initKnob(kTapeDrive, "tapeDrive", "DRIVE",   "",    kAmber);
    initKnob(kTapeBias,  "tapeBias",  "AC BIAS", "",    kAmber);

    // Module Bypass LEDs
    initBypass(bSub,    "enSub");
    initBypass(bTrans,  "enTrans");
    initBypass(bHead,   "enHead");
    initBypass(bComp,   "enComp");
    initBypass(bDust,   "enDust");
    initBypass(bStarve, "enStarve");
    initBypass(bCem,    "enCem");

    setSize(1060, 610);
    startTimerHz(30);
}

LoFiErV3AudioProcessorEditor::~LoFiErV3AudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void LoFiErV3AudioProcessorEditor::initKnob(KnobControl& k, const juce::String& paramID,
                                            const juce::String& name, const juce::String& suffix,
                                            juce::Colour accent, bool compact)
{
    k.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle(compact ? juce::Slider::NoTextBox : juce::Slider::TextBoxBelow, false, 60, 15);
    k.slider.setTextValueSuffix(suffix);
    k.slider.setColour(juce::Slider::rotarySliderFillColourId, accent);
    k.slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFFCDD5DF));
    k.slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    k.slider.setPopupDisplayEnabled(compact, false, this);
    addAndMakeVisible(k.slider);

    k.label.setText(name, juce::dontSendNotification);
    k.label.setJustificationType(juce::Justification::centred);
    k.label.setFont(juce::FontOptions(compact ? 9.5f : 10.0f, juce::Font::bold));
    k.label.setColour(juce::Label::textColourId, kTextMuted);
    addAndMakeVisible(k.label);

    k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, paramID, k.slider);
}

void LoFiErV3AudioProcessorEditor::initBypass(BypassLed& b, const juce::String& paramID)
{
    addAndMakeVisible(b.button);
    b.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, paramID, b.button);
}

void LoFiErV3AudioProcessorEditor::timerCallback()
{
    crtVisualizer.tick();

    const float inL  = audioProcessor.meterInL.load(std::memory_order_relaxed);
    const float inR  = audioProcessor.meterInR.load(std::memory_order_relaxed);
    const float outL = audioProcessor.meterOutL.load(std::memory_order_relaxed);
    const float outR = audioProcessor.meterOutR.load(std::memory_order_relaxed);

    uiInL  = std::max(inL,  uiInL  * 0.84f);
    uiInR  = std::max(inR,  uiInR  * 0.84f);
    uiOutL = std::max(outL, uiOutL * 0.84f);
    uiOutR = std::max(outR, uiOutR * 0.84f);
    uiDrop = std::max(audioProcessor.telDrop.load(std::memory_order_relaxed), uiDrop * 0.80f);

    repaint(inMeterBounds);
    repaint(outMeterBounds);
    repaint(cardBounds[5]);
}

void LoFiErV3AudioProcessorEditor::drawScrew(juce::Graphics& g, float cx, float cy) const
{
    const float r = 4.5f;
    g.setColour(juce::Colour(0xFF0A0C0E));
    g.fillEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f);
    g.setColour(juce::Colour(0xFF2D343E));
    g.fillEllipse(cx - r + 1.0f, cy - r + 1.0f, (r - 1.0f) * 2.0f, (r - 1.0f) * 2.0f);
    g.setColour(juce::Colour(0xFF121519));
    g.drawLine(cx - 2.3f, cy - 1.5f, cx + 2.3f, cy + 1.5f, 1.2f);
}

void LoFiErV3AudioProcessorEditor::drawStereoMeter(juce::Graphics& g, juce::Rectangle<int> r,
                                                   float lvlL, float lvlR, const juce::String& lbl) const
{
    g.setColour(juce::Colour(0xFF0B0D10));
    g.fillRoundedRectangle(r.toFloat(), 3.0f);
    g.setColour(kCardBorder);
    g.drawRoundedRectangle(r.toFloat(), 3.0f, 1.0f);

    auto inner = r.reduced(3, 3);
    auto labelArea = inner.removeFromLeft(22);
    g.setColour(kTextMuted);
    g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
    g.drawText(lbl, labelArea, juce::Justification::centredLeft);

    auto drawBar = [&](juce::Rectangle<int> bar, float lin)
    {
        const float db = juce::Decibels::gainToDecibels(std::max(lin, 1e-5f), -48.0f);
        const float norm = std::clamp((db + 48.0f) / 51.0f, 0.0f, 1.0f);
        const int segs = 12;
        const float segW = static_cast<float>(bar.getWidth()) / static_cast<float>(segs);

        for (int s = 0; s < segs; ++s)
        {
            const float thresh = static_cast<float>(s + 1) / static_cast<float>(segs);
            const bool lit = norm >= (thresh - 0.06f);
            juce::Colour col = (s >= 10) ? kCoral : ((s >= 7) ? kAmber : kPhosphor);
            g.setColour(lit ? col : col.withAlpha(0.12f));
            g.fillRect(static_cast<float>(bar.getX()) + static_cast<float>(s) * segW + 1.0f,
                       static_cast<float>(bar.getY()), segW - 2.0f, static_cast<float>(bar.getHeight()));
        }
    };

    const int h = (inner.getHeight() - 2) / 2;
    drawBar(inner.removeFromTop(h), lvlL);
    inner.removeFromTop(2);
    drawBar(inner.removeFromTop(h), lvlR);
}

void LoFiErV3AudioProcessorEditor::paint(juce::Graphics& g)
{
    // Brushed metallic console chassis
    juce::ColourGradient bg(kChassisTop, 0.0f, 0.0f, kChassisBot, 0.0f, static_cast<float>(getHeight()), false);
    g.setGradientFill(bg);
    g.fillAll();

    // Corner Rack Screws
    drawScrew(g, 11.0f, 11.0f);
    drawScrew(g, static_cast<float>(getWidth()) - 11.0f, 11.0f);
    drawScrew(g, 11.0f, static_cast<float>(getHeight()) - 11.0f);
    drawScrew(g, static_cast<float>(getWidth()) - 11.0f, static_cast<float>(getHeight()) - 11.0f);

    // Top Header Console Bar
    auto header = getLocalBounds().removeFromTop(54);
    g.setColour(juce::Colour(0xFF12151A));
    g.fillRect(header);
    g.setColour(kCardBorder);
    g.drawHorizontalLine(header.getBottom() - 1, 0.0f, static_cast<float>(getWidth()));

    // Brand Title + V3 Hardware Badge
    g.setColour(kTextBright);
    g.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    g.drawText("LO-FI-ER", 26, 8, 96, 22, juce::Justification::centredLeft);

    auto badge = juce::Rectangle<float>(118.0f, 11.0f, 28.0f, 16.0f);
    g.setColour(kPhosphor.withAlpha(0.18f));
    g.fillRoundedRectangle(badge, 3.0f);
    g.setColour(kPhosphor);
    g.drawRoundedRectangle(badge, 3.0f, 1.0f);
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.drawText("V3", badge, juce::Justification::centred);

    g.setColour(kTextMuted);
    g.setFont(juce::FontOptions(9.5f, juce::Font::plain));
    g.drawText("ANALOG & FAULT MASTERING CONSOLE", 26, 30, 210, 14, juce::Justification::centredLeft);

    // Stereo In / Out LED Meters
    drawStereoMeter(g, inMeterBounds,  uiInL,  uiInR,  "IN");
    drawStereoMeter(g, outMeterBounds, uiOutL, uiOutR, "OUT");

    // Module Cards
    static const char* titles[9] = {
        "01 // SUB ANCHOR", "09 // MASTER TAPE DECK",
        "02 // TRANSPORT",  "03 // TAPE HEAD", "04 // AD/DA COMPANDER",
        "05 // FAULT ENGINE", "06 // LIVING DUST", "07 // VOLTAGE STARVE", "08 // CEM3320 & WOOD"
    };

    for (int i = 0; i < 9; ++i)
    {
        auto r = cardBounds[i].toFloat();
        g.setColour(kCardBg);
        g.fillRoundedRectangle(r, 6.0f);

        // Card Header Strip
        auto hdr = r.withHeight(24.0f);
        g.setColour(kCardHeader);
        g.fillRoundedRectangle(hdr, 6.0f);
        g.fillRect(hdr.withTop(hdr.getBottom() - 6.0f));

        g.setColour(kCardBorder);
        g.drawRoundedRectangle(r, 6.0f, 1.0f);
        g.drawHorizontalLine(static_cast<int>(hdr.getBottom()), r.getX(), r.getRight());

        g.setColour(kPhosphor.withAlpha(0.90f));
        g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
        g.drawText(titles[i], cardBounds[i].withHeight(24).reduced(10, 0), juce::Justification::centredLeft);
    }

    // Fault Engine Active PKT LED
    const auto fCard = cardBounds[5];
    const float ledX = static_cast<float>(fCard.getRight() - 22);
    const float ledY = static_cast<float>(fCard.getY() + 7);
    g.setColour(uiDrop > 0.25f ? kCoral : juce::Colour(0xFF2B2020));
    g.fillEllipse(ledX, ledY, 10.0f, 10.0f);
}

void LoFiErV3AudioProcessorEditor::resized()
{
    auto full = getLocalBounds();
    auto header = full.removeFromTop(54);

    // Header: Presets in center-left, Meters + Master Knobs on right
    prevPresetBtn.setBounds(252, 14, 24, 26);
    presetBox.setBounds(280, 14, 200, 26);
    nextPresetBtn.setBounds(484, 14, 24, 26);

    inMeterBounds  = juce::Rectangle<int>(534, 14, 126, 26);
    outMeterBounds = juce::Rectangle<int>(668, 14, 126, 26);

    auto placeMiniKnob = [](KnobControl& k, int x)
    {
        k.label.setBounds(x, 4, 52, 12);
        k.slider.setBounds(x + 4, 15, 44, 36);
    };
    placeMiniKnob(kInGain,  822);
    placeMiniKnob(kOutGain, 886);
    placeMiniKnob(kMix,     950);

    // Main 3-row rack layout
    auto area = full.reduced(16, 12);
    const int gap = 10;
    const int r1H = 186;
    const int remH = area.getHeight() - r1H - gap * 2;
    const int r2H = remH / 2;

    auto row1 = area.removeFromTop(r1H);
    area.removeFromTop(gap);
    auto row2 = area.removeFromTop(r2H);
    area.removeFromTop(gap);
    auto row3 = area;

    // Row 1: [Sub Anchor 305px] | [P31 CRT Monitor flex] | [Master Tape Deck 305px]
    cardBounds[0] = row1.removeFromLeft(305);
    row1.removeFromLeft(gap);
    cardBounds[1] = row1.removeFromRight(305);
    row1.removeFromRight(gap);
    crtVisualizer.setBounds(row1);

    auto splitRow = [&](juce::Rectangle<int> row, std::initializer_list<int> weights, int startIdx)
    {
        int totalW = 0;
        for (int w : weights) totalW += w;
        const int avail = row.getWidth() - gap * (static_cast<int>(weights.size()) - 1);
        int idx = startIdx;
        for (int w : weights)
        {
            const int px = (avail * w) / totalW;
            cardBounds[idx++] = row.removeFromLeft(px);
            row.removeFromLeft(gap);
        }
    };

    // Row 2: Transport (3) | Tape Head (2) | Compander (4)
    splitRow(row2, { 3, 2, 4 }, 2);
    // Row 3: Fault (2) | Dust (2) | Starve (2) | CEM & Wood (3)
    splitRow(row3, { 2, 2, 2, 3 }, 5);

    auto layoutBypass = [](juce::Rectangle<int> card, BypassLed& b)
    {
        b.button.setBounds(card.getRight() - 26, card.getY() + 3, 20, 18);
    };

    auto layoutKnobs = [](juce::Rectangle<int> card, std::initializer_list<KnobControl*> knobs)
    {
        card.removeFromTop(28);
        card.reduce(8, 8);
        const int colW = card.getWidth() / static_cast<int>(knobs.size());
        for (auto* k : knobs)
        {
            auto col = card.removeFromLeft(colW);
            k->label.setBounds(col.removeFromTop(14));
            k->slider.setBounds(col.reduced(2, 2));
        }
    };

    // 01 // Sub Anchor (2x2 grid so it fits 305px cleanly)
    layoutBypass(cardBounds[0], bSub);
    {
        auto c = cardBounds[0];
        c.removeFromTop(26);
        c.reduce(8, 4);
        auto topHalf = c.removeFromTop(c.getHeight() / 2);
        auto botHalf = c;
        auto placePair = [](juce::Rectangle<int> r, KnobControl& a, KnobControl& b)
        {
            auto left = r.removeFromLeft(r.getWidth() / 2);
            a.label.setBounds(left.removeFromTop(12));
            a.slider.setBounds(left.reduced(6, 0));
            b.label.setBounds(r.removeFromTop(12));
            b.slider.setBounds(r.reduced(6, 0));
        };
        placePair(topHalf, kSubFc, kSubWarm);
        placePair(botHalf, kSubIron, kSubThump);
    }

    // 09 // Master Tape Deck (ComboBox top + 2 knobs below)
    {
        auto c = cardBounds[1];
        c.removeFromTop(30);
        c.reduce(12, 4);
        tapeModeBox.setBounds(c.removeFromTop(26));
        c.removeFromTop(8);
        const int halfW = c.getWidth() / 2;
        auto col1 = c.removeFromLeft(halfW);
        kTapeDrive.label.setBounds(col1.removeFromTop(14));
        kTapeDrive.slider.setBounds(col1.reduced(6, 2));
        kTapeBias.label.setBounds(c.removeFromTop(14));
        kTapeBias.slider.setBounds(c.reduced(6, 2));
    }

    // Row 2
    layoutBypass(cardBounds[2], bTrans);
    layoutKnobs(cardBounds[2], { &kWow, &kFlutter, &kDropouts });

    layoutBypass(cardBounds[3], bHead);
    layoutKnobs(cardBounds[3], { &kGapDrive, &kAzimuth });

    layoutBypass(cardBounds[4], bComp);
    layoutKnobs(cardBounds[4], { &kCompRate, &kCompBits, &kCompEmp, &kCompAmt });

    // Row 3
    {
        auto c = cardBounds[5];
        c.removeFromTop(28);
        faultBox.setBounds(c.reduced(14, 42));
    }

    layoutBypass(cardBounds[6], bDust);
    layoutKnobs(cardBounds[6], { &kHalo, &kVinyl });

    layoutBypass(cardBounds[7], bStarve);
    layoutKnobs(cardBounds[7], { &kStarve, &kDrag });

    layoutBypass(cardBounds[8], bCem);
    layoutKnobs(cardBounds[8], { &kCemFc, &kCemRes, &kWoodBox });
}
