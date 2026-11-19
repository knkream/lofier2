#include "PluginProcessor.h"
#include "PluginEditor.h"

const std::vector<PresetDef>& LoFiErV3AudioProcessor::getPresets()
{
    static const std::vector<PresetDef> presets = {
        // Name, inG, outG, mix | subFc, Warm, Iron, Thump | wow, flut, drop | gap, az | rate, bits, emp, comp | fault | halo, vinyl | starve, drag | cemFc, res, wood | tMode, tDrv, tBias | enables...
        { "01 // INIT CONSOLE",   0.0f,  0.0f, 1.00f,  110.f, 1.4f, 0.20f, 0.00f,  0.15f, 0.10f, 0.05f,  0.28f, 0.15f,  44100.f, 16.0f, 0.00f, 0.00f,  0,  0.18f, 0.00f,  0.00f, 0.00f,  18000.f, 0.00f, 0.00f,  1, 0.25f,  0.00f,  true, true, true, false, true, false, false },
        { "02 // SP-1200 GRIT",   1.5f, -0.5f, 1.00f,  105.f, 2.2f, 0.45f, 0.22f,  0.10f, 0.08f, 0.00f,  0.30f, 0.12f,  26040.f, 12.0f, 0.45f, 0.35f,  1,  0.12f, 0.15f,  0.08f, 0.00f,   7400.f, 0.32f, 0.18f,  4, 0.48f,  0.10f,  true, true, true, true,  true, true,  true  },
        { "03 // DYING WALKMAN",  0.5f,  0.5f, 1.00f,  125.f, 1.8f, 0.30f, 0.00f,  0.62f, 0.48f, 0.38f,  0.55f, 0.52f,  32000.f, 14.0f, 0.10f, 0.15f,  0,  0.48f, 0.08f,  0.52f, 0.58f,   6200.f, 0.18f, 0.32f,  3, 0.62f, -0.35f,  true, true, true, false, true, true,  true  },
        { "04 // 90S BOOM BAP",   2.0f, -1.0f, 1.00f,   95.f, 2.6f, 0.58f, 0.35f,  0.22f, 0.18f, 0.12f,  0.46f, 0.26f,  22050.f, 11.5f, 0.55f, 0.48f,  0,  0.30f, 0.42f,  0.12f, 0.08f,   9200.f, 0.22f, 0.25f,  2, 0.52f,  0.20f,  true, true, true, true,  true, true,  true  },
        { "05 // GSM GHOST",      0.0f,  0.0f, 0.90f,  130.f, 1.5f, 0.20f, 0.00f,  0.30f, 0.35f, 0.25f,  0.40f, 0.35f,  11025.f,  8.5f, 0.25f, 0.60f,  2,  0.35f, 0.00f,  0.35f, 0.22f,   4600.f, 0.45f, 0.45f,  5, 0.55f, -0.20f,  true, true, true, true,  true, true,  true  },
        { "06 // RAM ROT CHAOS",  1.0f, -1.5f, 0.85f,  115.f, 2.8f, 0.65f, 0.25f,  0.40f, 0.55f, 0.45f,  0.65f, 0.48f,   8200.f,  6.5f, 0.40f, 0.55f,  3,  0.42f, 0.28f,  0.65f, 0.42f,   5400.f, 0.55f, 0.38f,  5, 0.72f, -0.55f,  true, true, true, true,  true, true,  true  },
        { "07 // WARM MASTER",    0.5f,  0.0f, 1.00f,   85.f, 1.9f, 0.32f, 0.10f,  0.08f, 0.05f, 0.00f,  0.24f, 0.10f,  44100.f, 16.0f, 0.00f, 0.00f,  0,  0.14f, 0.00f,  0.00f, 0.00f,  16500.f, 0.08f, 0.12f,  1, 0.42f,  0.25f,  true, true, true, false, true, false, true  }
    };
    return presets;
}

LoFiErV3AudioProcessor::LoFiErV3AudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    pInGain    = apvts.getRawParameterValue("inGain");
    pOutGain   = apvts.getRawParameterValue("outGain");
    pMix       = apvts.getRawParameterValue("mix");

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

    pTapeMode  = apvts.getRawParameterValue("tapeMode");
    pTapeDrive = apvts.getRawParameterValue("tapeDrive");
    pTapeBias  = apvts.getRawParameterValue("tapeBias");

    pEnSub     = apvts.getRawParameterValue("enSub");
    pEnTrans   = apvts.getRawParameterValue("enTrans");
    pEnHead    = apvts.getRawParameterValue("enHead");
    pEnComp    = apvts.getRawParameterValue("enComp");
    pEnDust    = apvts.getRawParameterValue("enDust");
    pEnStarve  = apvts.getRawParameterValue("enStarve");
    pEnCem     = apvts.getRawParameterValue("enCem");
}

juce::AudioProcessorValueTreeState::ParameterLayout LoFiErV3AudioProcessor::createParameterLayout()
{
    using PFloat  = juce::AudioParameterFloat;
    using PChoice = juce::AudioParameterChoice;
    using PBool   = juce::AudioParameterBool;
    using Range   = juce::NormalisableRange<float>;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    // Master Console
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"inGain", 1},   "Input Gain",  Range(-12.f, 12.f, 0.1f), 0.0f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"outGain", 1},  "Output Gain", Range(-12.f, 12.f, 0.1f), 0.0f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"mix", 1},      "Mix",         Range(0.f, 1.f, 0.001f), 1.0f));

    // 01 // Sub Anchor
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"subFc", 1},    "Sub X-Over",  Range(30.f, 250.f, 1.f, 0.5f), 110.f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"subWarm", 1},  "Sub Warmth",  Range(1.f, 5.f, 0.01f), 1.4f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"subIron", 1},  "Sub Iron",    Range(0.f, 1.f, 0.001f), 0.20f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"subThump", 1}, "Sub Thump",   Range(0.f, 1.f, 0.001f), 0.0f));
    p.push_back(std::make_unique<PBool> (juce::ParameterID{"enSub", 1},    "Sub On",      true));

    // 02 // Transport
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"wow", 1},      "Wow",         Range(0.f, 1.f, 0.001f), 0.15f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"flutter", 1},  "Flutter",     Range(0.f, 1.f, 0.001f), 0.10f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"dropouts", 1}, "Dropouts",    Range(0.f, 1.f, 0.001f), 0.05f));
    p.push_back(std::make_unique<PBool> (juce::ParameterID{"enTrans", 1},  "Trans On",    true));

    // 03 // Tape Head
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"gapDrive", 1}, "Gap Drive",   Range(0.f, 1.f, 0.001f), 0.28f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"azimuth", 1},  "Azimuth",     Range(0.f, 1.f, 0.001f), 0.15f));
    p.push_back(std::make_unique<PBool> (juce::ParameterID{"enHead", 1},   "Head On",     true));

    // 04 // Compander
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"compRate", 1}, "Clock Hz",    Range(1500.f, 44100.f, 1.f, 0.35f), 44100.f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"compBits", 1}, "Bit Depth",   Range(4.f, 16.f, 0.1f), 16.f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"compEmp", 1},  "Pre-Emp",     Range(0.f, 1.f, 0.001f), 0.0f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"compAmt", 1},  "Compander",   Range(0.f, 1.f, 0.001f), 0.0f));
    p.push_back(std::make_unique<PBool> (juce::ParameterID{"enComp", 1},   "Comp On",     false));

    // 05 // Fault Engine
    p.push_back(std::make_unique<PChoice>(juce::ParameterID{"faultMode", 1}, "Fault Mode",
        juce::StringArray{"OFF", "DAC DRIFT", "GSM 1-BAR", "RAM ROT"}, 0));

    // 06 // Living Dust
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"halo", 1},     "Tape Halo",   Range(0.f, 1.f, 0.001f), 0.18f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"vinyl", 1},    "Vinyl Dust",  Range(0.f, 1.f, 0.001f), 0.0f));
    p.push_back(std::make_unique<PBool> (juce::ParameterID{"enDust", 1},   "Dust On",     true));

    // 07 // Voltage Starve
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"starve", 1},   "Starve",      Range(0.f, 1.f, 0.001f), 0.0f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"drag", 1},     "Motor Drag",  Range(0.f, 1.f, 0.001f), 0.0f));
    p.push_back(std::make_unique<PBool> (juce::ParameterID{"enStarve", 1}, "Starve On",   false));

    // 08 // CEM3320 & Wood Box
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"cemFc", 1},    "CEM Cutoff",  Range(800.f, 18000.f, 1.f, 0.35f), 18000.f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"cemRes", 1},   "CEM Res",     Range(0.f, 0.85f, 0.001f), 0.0f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"woodBox", 1},  "Wood Box",    Range(0.f, 1.f, 0.001f), 0.0f));
    p.push_back(std::make_unique<PBool> (juce::ParameterID{"enCem", 1},    "CEM On",      false));

    // 09 // Master Tape Machine
    p.push_back(std::make_unique<PChoice>(juce::ParameterID{"tapeMode", 1}, "Tape Machine",
        juce::StringArray{"BYPASS", "STUDIO 15IPS", "PRO 7.5IPS", "CASSETTE", "HEAVY IRON", "AGED REEL"}, 1));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"tapeDrive", 1}, "Tape Drive", Range(0.f, 1.f, 0.001f), 0.25f));
    p.push_back(std::make_unique<PFloat>(juce::ParameterID{"tapeBias", 1},  "AC Bias",    Range(-1.f, 1.f, 0.01f), 0.0f));

    return { p.begin(), p.end() };
}

void LoFiErV3AudioProcessor::setCurrentProgram(int index)
{
    applyPreset(index);
}

const juce::String LoFiErV3AudioProcessor::getProgramName(int index)
{
    const auto& presets = getPresets();
    if (index >= 0 && index < static_cast<int>(presets.size()))
        return presets[static_cast<size_t>(index)].name;
    return {};
}

void LoFiErV3AudioProcessor::applyPreset(int index)
{
    const auto& presets = getPresets();
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return;

    currentPresetIndex = index;
    const auto& pr = presets[static_cast<size_t>(index)];

    auto setParam = [&](const char* id, float val)
    {
        if (auto* param = apvts.getParameter(id))
            param->setValueNotifyingHost(param->convertTo0to1(val));
    };

    setParam("inGain",    pr.inGain);
    setParam("outGain",   pr.outGain);
    setParam("mix",       pr.mix);
    setParam("subFc",     pr.subFc);
    setParam("subWarm",   pr.subWarm);
    setParam("subIron",   pr.subIron);
    setParam("subThump",  pr.subThump);
    setParam("wow",       pr.wow);
    setParam("flutter",   pr.flutter);
    setParam("dropouts",  pr.dropouts);
    setParam("gapDrive",  pr.gapDrive);
    setParam("azimuth",   pr.azimuth);
    setParam("compRate",  pr.compRate);
    setParam("compBits",  pr.compBits);
    setParam("compEmp",   pr.compEmp);
    setParam("compAmt",   pr.compAmt);
    setParam("faultMode", static_cast<float>(pr.faultMode));
    setParam("halo",      pr.halo);
    setParam("vinyl",     pr.vinyl);
    setParam("starve",    pr.starve);
    setParam("drag",      pr.drag);
    setParam("cemFc",     pr.cemFc);
    setParam("cemRes",    pr.cemRes);
    setParam("woodBox",   pr.woodBox);
    setParam("tapeMode",  static_cast<float>(pr.tapeMode));
    setParam("tapeDrive", pr.tapeDrive);
    setParam("tapeBias",  pr.tapeBias);
    setParam("enSub",     pr.enSub    ? 1.0f : 0.0f);
    setParam("enTrans",   pr.enTrans  ? 1.0f : 0.0f);
    setParam("enHead",    pr.enHead   ? 1.0f : 0.0f);
    setParam("enComp",    pr.enComp   ? 1.0f : 0.0f);
    setParam("enDust",    pr.enDust   ? 1.0f : 0.0f);
    setParam("enStarve",  pr.enStarve ? 1.0f : 0.0f);
    setParam("enCem",     pr.enCem    ? 1.0f : 0.0f);
}

void LoFiErV3AudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
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
    masterTape.prepare(currentSampleRate);

    auto resetSm = [&](juce::SmoothedValue<double>& sm, double val)
    {
        sm.reset(currentSampleRate, 0.015);
        sm.setCurrentAndTargetValue(val);
    };

    resetSm(smInGain,    juce::Decibels::decibelsToGain(static_cast<double>(pInGain->load())));
    resetSm(smOutGain,   juce::Decibels::decibelsToGain(static_cast<double>(pOutGain->load())));
    resetSm(smMix,       pMix->load());
    resetSm(smSubFc,     pSubFc->load());
    resetSm(smSubWarm,   pSubWarm->load());
    resetSm(smSubIron,   pSubIron->load());
    resetSm(smSubThump,  pSubThump->load());
    resetSm(smWow,       pWow->load());
    resetSm(smFlutter,   pFlutter->load());
    resetSm(smDropouts,  pDropouts->load());
    resetSm(smGapDrive,  pGapDrive->load());
    resetSm(smAzimuth,   pAzimuth->load());
    resetSm(smCompRate,  pCompRate->load());
    resetSm(smCompBits,  pCompBits->load());
    resetSm(smCompEmp,   pCompEmp->load());
    resetSm(smCompAmt,   pCompAmt->load());
    resetSm(smHalo,      pHalo->load());
    resetSm(smVinyl,     pVinyl->load());
    resetSm(smStarve,    pStarve->load());
    resetSm(smDrag,      pDrag->load());
    resetSm(smCemFc,     pCemFc->load());
    resetSm(smCemRes,    pCemRes->load());
    resetSm(smWoodBox,   pWoodBox->load());
    resetSm(smTapeDrive, pTapeDrive->load());
    resetSm(smTapeBias,  pTapeBias->load());

    for (auto& s : scopeRing) s.store(0.0f);
    scopeWritePos.store(0);
    scopeDecimCounter = 0;
}

bool LoFiErV3AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void LoFiErV3AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    auto* left  = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    const double sr = currentSampleRate;

    // Update smoothed targets
    smInGain.setTargetValue(juce::Decibels::decibelsToGain(static_cast<double>(pInGain->load())));
    smOutGain.setTargetValue(juce::Decibels::decibelsToGain(static_cast<double>(pOutGain->load())));
    smMix.setTargetValue(pMix->load());
    smSubFc.setTargetValue(pSubFc->load());
    smSubWarm.setTargetValue(pSubWarm->load());
    smSubIron.setTargetValue(pSubIron->load());
    smSubThump.setTargetValue(pSubThump->load());
    smWow.setTargetValue(pWow->load());
    smFlutter.setTargetValue(pFlutter->load());
    smDropouts.setTargetValue(pDropouts->load());
    smGapDrive.setTargetValue(pGapDrive->load());
    smAzimuth.setTargetValue(pAzimuth->load());
    smCompRate.setTargetValue(pCompRate->load());
    smCompBits.setTargetValue(pCompBits->load());
    smCompEmp.setTargetValue(pCompEmp->load());
    smCompAmt.setTargetValue(pCompAmt->load());
    smHalo.setTargetValue(pHalo->load());
    smVinyl.setTargetValue(pVinyl->load());
    smStarve.setTargetValue(pStarve->load());
    smDrag.setTargetValue(pDrag->load());
    smCemFc.setTargetValue(pCemFc->load());
    smCemRes.setTargetValue(pCemRes->load());
    smWoodBox.setTargetValue(pWoodBox->load());
    smTapeDrive.setTargetValue(pTapeDrive->load());
    smTapeBias.setTargetValue(pTapeBias->load());

    const int  faultMode = static_cast<int>(std::round(pFaultMode->load()));
    const int  tapeMode  = static_cast<int>(std::round(pTapeMode->load()));
    const bool enSub     = pEnSub->load()    > 0.5f;
    const bool enTrans   = pEnTrans->load()  > 0.5f;
    const bool enHead    = pEnHead->load()   > 0.5f;
    const bool enComp    = pEnComp->load()   > 0.5f;
    const bool enDust    = pEnDust->load()   > 0.5f;
    const bool enStarve  = pEnStarve->load() > 0.5f;
    const bool enCem     = pEnCem->load()    > 0.5f;

    double pkInL = 0.0, pkInR = 0.0, pkOutL = 0.0, pkOutR = 0.0;
    double maxEnv = 0.0, maxTrans = 0.0, maxDrop = 0.0, maxSag = 0.0;
    const int decimStep = std::max(1, static_cast<int>(std::round(sr / 3000.0)));

    for (int i = 0; i < numSamples; ++i)
    {
        const double inGain   = smInGain.getNextValue();
        const double outGain  = smOutGain.getNextValue();
        const double wetMix   = smMix.getNextValue();

        const double subFc    = smSubFc.getNextValue();
        const double subWarm  = smSubWarm.getNextValue();
        const double subIronV = smSubIron.getNextValue();
        const double subThump = smSubThump.getNextValue();

        const double wow      = smWow.getNextValue();
        const double flutter  = smFlutter.getNextValue();
        const double dropouts = smDropouts.getNextValue();

        const double gapDrive = smGapDrive.getNextValue();
        const double azimuth  = smAzimuth.getNextValue();

        const double compRate = smCompRate.getNextValue();
        const double compBits = smCompBits.getNextValue();
        const double compEmp  = smCompEmp.getNextValue();
        const double compAmt  = smCompAmt.getNextValue();

        const double halo     = smHalo.getNextValue();
        const double vinyl    = smVinyl.getNextValue();

        const double starve   = smStarve.getNextValue();
        const double drag     = smDrag.getNextValue();

        const double cemFc    = smCemFc.getNextValue();
        const double cemRes   = smCemRes.getNextValue();
        const double woodBox  = smWoodBox.getNextValue();

        const double tDrive   = smTapeDrive.getNextValue();
        const double tBias    = smTapeBias.getNextValue();

        const double dryL = static_cast<double>(left[i])  * inGain;
        const double dryR = static_cast<double>(right[i]) * inGain;

        pkInL = std::max(pkInL, std::abs(dryL));
        pkInR = std::max(pkInR, std::abs(dryR));

        // Internal Envelope & Transient Telemetry
        double env = 0.0, trans = 0.0;
        tracker.process(dryL, dryR, sr, env, trans);
        if (env > maxEnv) maxEnv = env;
        if (trans > maxTrans) maxTrans = trans;

        // 01 // LR4 Sub-Anchor Split & Sub Iron
        double subL = 0.0, highL = dryL, subR = 0.0, highR = dryR;
        if (enSub)
        {
            xoverL.process(dryL, subFc, subWarm, sr, subL, highL);
            xoverR.process(dryR, subFc, subWarm, sr, subR, highR);
            subIron.process(subL, subR, trans, subIronV, subThump, sr);
        }
        const double subSidechain = (subL + subR) * 0.5;

        // 02..08 // High-Band Lo-Fi Chain
        if (enTrans)  transport.process(highL, highR, wow, flutter, dropouts, env, trans, sr);
        if (enHead)   tapeHead.process(highL, highR, subSidechain, gapDrive, azimuth, sr);
        if (enComp)   compander.process(highL, highR, env, compRate, compBits, compEmp, compAmt, sr);

        if (faultMode > 0)
        {
            const double frz = faultEngine.process(highL, highR, env, faultMode, sr);
            if (frz > maxDrop) maxDrop = frz;
        }

        if (enDust)   livingDust.process(highL, highR, env, halo, vinyl, sr);

        if (enStarve)
        {
            const double sag = voltageStarve.process(highL, highR, env, starve, drag, sr);
            if (sag > maxSag) maxSag = sag;
        }

        if (enCem)    cemWood.process(highL, highR, trans, cemFc, cemRes, woodBox, sr);

        // Recombine Sub + High
        double wetL = subL + highL;
        double wetR = subR + highR;

        // 09 // Post-Chain Master Tape Machine
        if (tapeMode > 0)
            masterTape.process(wetL, wetR, tapeMode, tDrive, tBias, sr);

        // Global Mix + Output Trim
        const double outL = lofier::mix(dryL, wetL, wetMix) * outGain;
        const double outR = lofier::mix(dryR, wetR, wetMix) * outGain;

        pkOutL = std::max(pkOutL, std::abs(outL));
        pkOutR = std::max(pkOutR, std::abs(outR));

        left[i]  = static_cast<float>(outL);
        right[i] = static_cast<float>(outR);

        // Feed CRT Oscilloscope Ring Buffer
        if (++scopeDecimCounter >= decimStep)
        {
            scopeDecimCounter = 0;
            const int wp = scopeWritePos.load(std::memory_order_relaxed);
            scopeRing[static_cast<size_t>(wp)].store(static_cast<float>((outL + outR) * 0.5), std::memory_order_relaxed);
            scopeWritePos.store((wp + 1) % kScopeSize, std::memory_order_release);
        }
    }

    telEnv.store(static_cast<float>(maxEnv), std::memory_order_relaxed);
    telTrans.store(static_cast<float>(maxTrans), std::memory_order_relaxed);
    telDrop.store(static_cast<float>(maxDrop), std::memory_order_relaxed);
    telSag.store(static_cast<float>(maxSag), std::memory_order_relaxed);
    telBits.store(enComp ? pCompBits->load() : 16.0f, std::memory_order_relaxed);
    telSr.store(enComp ? pCompRate->load() : static_cast<float>(sr), std::memory_order_relaxed);

    meterInL.store(static_cast<float>(pkInL), std::memory_order_relaxed);
    meterInR.store(static_cast<float>(pkInR), std::memory_order_relaxed);
    meterOutL.store(static_cast<float>(pkOutL), std::memory_order_relaxed);
    meterOutR.store(static_cast<float>(pkOutR), std::memory_order_relaxed);
}

juce::AudioProcessorEditor* LoFiErV3AudioProcessor::createEditor()
{
    return new LoFiErV3AudioProcessorEditor(*this);
}

void LoFiErV3AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void LoFiErV3AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LoFiErV3AudioProcessor();
}
