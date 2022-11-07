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

    // Give DSP initial values
    void init();

    // Pass sample rate and buffer size to DSP 
    void prepare(double sampleRate, int samplesPerBlock);

    // Called when user changes parameters
    void update();

    // Reset DSP parameters
    void reset() override;

    // Store Parameters
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

private:

    bool isActive{ false };
    bool mustUpdateProcessing{ false };

    juce::LinearSmoothedValue<float> driveNormal{ 0.0 };
    juce::LinearSmoothedValue<float> outputVolume[2]{ 0.0 };
    juce::LinearSmoothedValue<float> outputMix[2]{ 0.0 };

    // Called when user changes a parameter
    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override
    {
        mustUpdateProcessing = true;
    }
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginTemplateAudioProcessor)
};
