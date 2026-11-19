#include "PluginProcessor.h"
#include "PluginEditor.h"

LoFiErV2AudioProcessor::LoFiErV2AudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    pSubFc     = apvts.getRawParameterValue("subFc");
    pSubWarm   = apvts.getRawParameterValue("subWarm");
    pSubIron   = apvts.getRawParameterValue("subIron");
    pSubThump  = apvts.getRawParameterValue("subThump");

    pWow       = apvts.getRawParameterValue("wow");
    pFlutter   = apvts.getRawParameterValue("flutter");
    pDropouts  = apvts.getRawParameterValue("dropouts");

    pGapDrive  = apvts.getRawParameterValue("gapDrive");
    pAzimuth   = apvts.getRawParameterValue("azimuth");

    pCompRate  = apvts.getRawParameterValue("compRate");
    pCompBits  = apvts.getRawParameterValue("compBits");
    pCompEmp   = apvts.getRawParameterValue("compEmp");
    pCompAmt   = apvts.getRawParameterValue("compAmt");

    pFaultMode = apvts.getRawParameterValue("faultMode");

    pHalo      = apvts.getRawParameterValue("halo");
    pVinyl     = apvts.getRawParameterValue("vinyl");

    pStarve    = apvts.getRawParameterValue("starve");
    pDrag      = apvts.getRawParameterValue("drag");

    pCemFc     = apvts.getRawParameterValue("cemFc");
    pCemRes    = apvts.getRawParameterValue("cemRes");
    pWoodBox   = apvts.getRawParameterValue("woodBox");
    pMix       = apvts.getRawParameterValue("mix");
}

juce::AudioProcessorValueTreeState::ParameterLayout LoFiErV2AudioProcessor::createParameterLayout()
{
    using PFloat  = juce::AudioParameterFloat;
    using PChoice = juce::AudioParameterChoice;
    using Range   = juce::NormalisableRange<float>;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    // 01 // Sub Anchor
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"subFc", 1},    "Sub X-Over", Range(30.f, 250.f, 1.f, 0.5f), 110.f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"subWarm", 1},  "Sub Warmth", Range(1.f, 5.f, 0.01f), 1.5f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"subIron", 1},  "Sub Iron",   Range(0.f, 1.f, 0.001f), 0.25f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"subThump", 1}, "Sub Thump",  Range(0.f, 1.f, 0.001f), 0.0f));

    // 02 // Transport
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"wow", 1},      "Wow",        Range(0.f, 1.f, 0.001f), 0.2f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"flutter", 1},  "Flutter",    Range(0.f, 1.f, 0.001f), 0.15f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"dropouts", 1}, "Dropouts",   Range(0.f, 1.f, 0.001f), 0.1f));

    // 03 // Tape Head
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"gapDrive", 1}, "Gap Drive",  Range(0.f, 1.f, 0.001f), 0.35f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"azimuth", 1},  "Azimuth",    Range(0.f, 1.f, 0.001f), 0.2f));

    // 04 // Compander
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"compRate", 1}, "Clock Hz",   Range(1500.f, 44100.f, 1.f, 0.35f), 44100.f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"compBits", 1}, "Bit Depth",  Range(4.f, 16.f, 0.1f), 16.f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"compEmp", 1},  "Pre-Emp",    Range(0.f, 1.f, 0.001f), 0.0f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"compAmt", 1},  "Compander",  Range(0.f, 1.f, 0.001f), 0.0f));

    // 05 // Fault Engine
    p.push_back(std::make_unique<PChoice>(juce::ParameterID{"faultMode", 1}, "Fault Mode",
        juce::StringArray{"OFF", "DAC DRIFT", "GSM 1-BAR", "RAM ROT"}, 0));

    // 06 // Living Dust
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"halo", 1},     "Tape Halo",  Range(0.f, 1.f, 0.001f), 0.2f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"vinyl", 1},    "Vinyl Dust", Range(0.f, 1.f, 0.001f), 0.0f));

    // 07 // Voltage Starve
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"starve", 1},   "Starve",     Range(0.f, 1.f, 0.001f), 0.0f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"drag", 1},     "Motor Drag", Range(0.f, 1.f, 0.001f), 0.0f));

    // 08 // CEM3320 & Output
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"cemFc", 1},    "CEM Cutoff", Range(800.f, 18000.f, 1.f, 0.35f), 18000.f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"cemRes", 1},   "CEM Res",    Range(0.f, 0.85f, 0.001f), 0.0f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"woodBox", 1},  "Wood Box",   Range(0.f, 1.f, 0.001f), 0.0f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"mix", 1},      "Mix",        Range(0.f, 1.f, 0.001f), 1.0f));

    return { p.begin(), p.end() };
}

void LoFiErV2AudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    tracker.reset();
    xoverL.reset();
    xoverR.reset();
    subIron.reset();
    transport.prepare(currentSampleRate);
    tapeHead.prepare(currentSampleRate);
    compander.reset();
    faultEngine.prepare();
    livingDust.prepare();
    voltageStarve.prepare(currentSampleRate);
    cemWood.prepare(currentSampleRate);
}

bool LoFiErV2AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void LoFiErV2AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    auto* left  = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    const double sr = currentSampleRate;

    const double subFc     = pSubFc->load();
    const double subWarm   = pSubWarm->load();
    const double subIronV  = pSubIron->load();
    const double subThump  = pSubThump->load();

    const double wow       = pWow->load();
    const double flutter   = pFlutter->load();
    const double dropouts  = pDropouts->load();

    const double gapDrive  = pGapDrive->load();
    const double azimuth   = pAzimuth->load();

    const double compRate  = pCompRate->load();
    const double compBits  = pCompBits->load();
    const double compEmp   = pCompEmp->load();
    const double compAmt   = pCompAmt->load();

    const int    faultMode = static_cast<int>(std::round(pFaultMode->load()));

    const double halo      = pHalo->load();
    const double vinyl     = pVinyl->load();

    const double starve    = pStarve->load();
    const double drag      = pDrag->load();

    const double cemFc     = pCemFc->load();
    const double cemRes    = pCemRes->load();
    const double woodBox   = pWoodBox->load();
    const double wetMix    = pMix->load();

    double maxFreeze = 0.0;
    double maxSag    = 0.0;

    for (int i = 0; i < numSamples; ++i)
    {
        const double dryL = static_cast<double>(left[i]);
        const double dryR = static_cast<double>(right[i]);

        // Internal dynamic telemetry
        double env = 0.0, trans = 0.0;
        tracker.process(dryL, dryR, sr, env, trans);

        // 01 // LR4 Sub-Anchor Split
        double subL = 0.0, highL = 0.0, subR = 0.0, highR = 0.0;
        xoverL.process(dryL, subFc, subWarm, sr, subL, highL);
        xoverR.process(dryR, subFc, subWarm, sr, subR, highR);

        // Sub Path: Iron + Octave Thump
        subIron.process(subL, subR, trans, subIronV, subThump, sr);
        const double subSidechain = (subL + subR) * 0.5;

        // High Path: Full Lo-Fi Chain
        transport.process(highL, highR, wow, flutter, dropouts, env, trans, sr);
        tapeHead.process(highL, highR, subSidechain, gapDrive, azimuth, sr);
        compander.process(highL, highR, env, compRate, compBits, compEmp, compAmt, sr);

        const double frz = faultEngine.process(highL, highR, env, faultMode, sr);
        if (frz > maxFreeze) maxFreeze = frz;

        livingDust.process(highL, highR, env, halo, vinyl, sr);

        const double sag = voltageStarve.process(highL, highR, env, starve, drag, sr);
        if (sag > maxSag) maxSag = sag;

        cemWood.process(highL, highR, trans, cemFc, cemRes, woodBox, sr);

        // Recombine Sub + High & apply Global Mix
        const double outL = lofier::mix(dryL, subL + highL, wetMix);
        const double outR = lofier::mix(dryR, subR + highR, wetMix);

        left[i]  = static_cast<float>(outL);
        right[i] = static_cast<float>(outR);
    }

    telemetryFreeze.store(static_cast<float>(maxFreeze));
    telemetrySag.store(static_cast<float>(maxSag));
}

juce::AudioProcessorEditor* LoFiErV2AudioProcessor::createEditor()
{
    return new LoFiErV2AudioProcessorEditor(*this);
}

void LoFiErV2AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void LoFiErV2AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LoFiErV2AudioProcessor();
}
