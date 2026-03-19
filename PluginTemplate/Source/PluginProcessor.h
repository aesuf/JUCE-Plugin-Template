/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class PluginTemplateAudioProcessor  : public juce::AudioProcessor,
                                      public juce::ValueTree::Listener
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    PluginTemplateAudioProcessor();
    ~PluginTemplateAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================

    // Reset DSP parameters
    void reset() override;

    // Store Parameters
    juce::AudioProcessorValueTreeState apvts;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginTemplateAudioProcessor)

    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    // Internal DSP lifecycle — called only from prepareToPlay / processBlock
    void init();
    void prepare(double sampleRate, int samplesPerBlock);
    void update();

    // Thread-safe flags (shared between audio thread and message thread)
    std::atomic<bool> isActive { false };
    std::atomic<bool> mustUpdateProcessing { false };

    // Called when user changes a parameter (message thread -> audio thread signal)
    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override
    {
        mustUpdateProcessing.store(true, std::memory_order_release);
    }

    // Cached raw parameter pointers (looked up once in constructor, lock-free reads)
    std::atomic<float>* driveParam = nullptr;
    std::atomic<float>* volParam   = nullptr;
    std::atomic<float>* mixParam   = nullptr;

    // Smoothed parameter values (advanced per-sample in processBlock)
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> driveSmoothed  { 0.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> volumeSmoothed { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoothed    { 0.0f };
};
