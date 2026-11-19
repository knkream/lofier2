#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <atomic>
#include "DspModules.h"

struct PresetDef
{
    const char* name;
    float inGain, outGain, mix;
    float subFc, subWarm, subIron, subThump;
    float wow, flutter, dropouts;
    float gapDrive, azimuth;
    float compRate, compBits, compEmp, compAmt;
    int   faultMode;
    float halo, vinyl;
    float starve, drag;
    float cemFc, cemRes, woodBox;
    int   tapeMode;
    float tapeDrive, tapeBias;
    bool  enSub, enTrans, enHead, enComp, enDust, enStarve, enCem;
};

class LoFiErV3AudioProcessor : public juce::AudioProcessor
{
public:
    LoFiErV3AudioProcessor();
    ~LoFiErV3AudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }

    int getNumPrograms() override { return static_cast<int>(getPresets().size()); }
    int getCurrentProgram() override { return currentPresetIndex; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    static const std::vector<PresetDef>& getPresets();
    void applyPreset(int index);

    juce::AudioProcessorValueTreeState apvts;

    // Lock-free CRT Visualizer & Metering Telemetry
    static constexpr int kScopeSize = 128;
    std::array<std::atomic<float>, kScopeSize> scopeRing {};
    std::atomic<int>   scopeWritePos { 0 };

    std::atomic<float> telEnv        { 0.0f };
    std::atomic<float> telTrans      { 0.0f };
    std::atomic<float> telDrop       { 0.0f };
    std::atomic<float> telSag        { 0.0f };
    std::atomic<float> telBits       { 16.0f };
    std::atomic<float> telSr         { 44100.0f };
    std::atomic<float> meterInL      { 0.0f };
    std::atomic<float> meterInR      { 0.0f };
    std::atomic<float> meterOutL     { 0.0f };
    std::atomic<float> meterOutR     { 0.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    lofier::EnvTransientTracker tracker;
    lofier::SubCrossoverMono    xoverL, xoverR;
    lofier::SubIronModule       subIron;
    lofier::TransportModule     transport;
    lofier::TapeHeadModule      tapeHead;
    lofier::CompanderModule     compander;
    lofier::FaultEngineModule   faultEngine;
    lofier::LivingDustModule    livingDust;
    lofier::VoltageStarveModule voltageStarve;
    lofier::CemWoodModule       cemWood;
    lofier::MasterTapeModule    masterTape;

    double currentSampleRate = 44100.0;
    int currentPresetIndex = 0;
    int scopeDecimCounter = 0;

    // Smoothed continuous parameters (eliminates zipper noise)
    juce::SmoothedValue<double> smInGain, smOutGain, smMix;
    juce::SmoothedValue<double> smSubFc, smSubWarm, smSubIron, smSubThump;
    juce::SmoothedValue<double> smWow, smFlutter, smDropouts;
    juce::SmoothedValue<double> smGapDrive, smAzimuth;
    juce::SmoothedValue<double> smCompRate, smCompBits, smCompEmp, smCompAmt;
    juce::SmoothedValue<double> smHalo, smVinyl;
    juce::SmoothedValue<double> smStarve, smDrag;
    juce::SmoothedValue<double> smCemFc, smCemRes, smWoodBox;
    juce::SmoothedValue<double> smTapeDrive, smTapeBias;

    // Raw atomic parameter pointers
    std::atomic<float> *pInGain, *pOutGain, *pMix;
    std::atomic<float> *pSubFc, *pSubWarm, *pSubIron, *pSubThump;
    std::atomic<float> *pWow, *pFlutter, *pDropouts;
    std::atomic<float> *pGapDrive, *pAzimuth;
    std::atomic<float> *pCompRate, *pCompBits, *pCompEmp, *pCompAmt;
    std::atomic<float> *pFaultMode;
    std::atomic<float> *pHalo, *pVinyl;
    std::atomic<float> *pStarve, *pDrag;
    std::atomic<float> *pCemFc, *pCemRes, *pWoodBox;
    std::atomic<float> *pTapeMode, *pTapeDrive, *pTapeBias;
    std::atomic<float> *pEnSub, *pEnTrans, *pEnHead, *pEnComp, *pEnDust, *pEnStarve, *pEnCem;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoFiErV3AudioProcessor)
};
