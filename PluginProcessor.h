#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DspModules.h"

class LoFiErV2AudioProcessor : public juce::AudioProcessor
{
public:
    LoFiErV2AudioProcessor();
    ~LoFiErV2AudioProcessor() override = default;

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

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // Live telemetry for UI LEDs
    std::atomic<float> telemetryFreeze { 0.0f };
    std::atomic<float> telemetrySag    { 0.0f };

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

    double currentSampleRate = 44100.0;

    // Parameter pointers
    std::atomic<float> *pSubFc, *pSubWarm, *pSubIron, *pSubThump;
    std::atomic<float> *pWow, *pFlutter, *pDropouts;
    std::atomic<float> *pGapDrive, *pAzimuth;
    std::atomic<float> *pCompRate, *pCompBits, *pCompEmp, *pCompAmt;
    std::atomic<float> *pFaultMode;
    std::atomic<float> *pHalo, *pVinyl;
    std::atomic<float> *pStarve, *pDrag;
    std::atomic<float> *pCemFc, *pCemRes, *pWoodBox, *pMix;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoFiErV2AudioProcessor)
};
