/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginTemplateAudioProcessor::PluginTemplateAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)

                     #endif
                       ), apvts(*this, nullptr, "Parameters", createParameters())
#endif
{
    // Cache raw parameter pointers — one-time string lookup, lock-free reads thereafter
    driveParam = apvts.getRawParameterValue("DRIVE");
    volParam   = apvts.getRawParameterValue("VOL");
    mixParam   = apvts.getRawParameterValue("MIX");

    apvts.state.addListener(this);
    init();
}

PluginTemplateAudioProcessor::~PluginTemplateAudioProcessor()
{
}

//==============================================================================
const juce::String PluginTemplateAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginTemplateAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginTemplateAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginTemplateAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginTemplateAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginTemplateAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginTemplateAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PluginTemplateAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PluginTemplateAudioProcessor::getProgramName (int index)
{
    return {};
}

void PluginTemplateAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PluginTemplateAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    isActive.store(true, std::memory_order_release);
    prepare(sampleRate, samplesPerBlock);
    reset();   // Set ramp times and snap smoothers to current targets
    update();  // Load APVTS parameter values as smoothed targets

    // Snap all smoothed values so the first buffer doesn't ramp from defaults
    driveSmoothed.setCurrentAndTargetValue(driveSmoothed.getTargetValue());
    volumeSmoothed.setCurrentAndTargetValue(volumeSmoothed.getTargetValue());
    mixSmoothed.setCurrentAndTargetValue(mixSmoothed.getTargetValue());
}

void PluginTemplateAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PluginTemplateAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PluginTemplateAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    if (!isActive.load(std::memory_order_acquire))
        return;

    if (mustUpdateProcessing.load(std::memory_order_acquire))
        update();

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    auto numSamples = buffer.getNumSamples();
    auto numChannels = juce::jmin(totalNumInputChannels, totalNumOutputChannels);

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);

    // Sub-block processing: advance smoothed parameters in fixed-size chunks.
    // When you add filters, recalculate their coefficients once per sub-block
    // (every 32 samples) instead of every sample — the standard pattern for
    // balancing smoothness against CPU cost.
    const int subBlockSize = 32;
    int samplesRemaining = numSamples;
    int startSample = 0;

    while (samplesRemaining > 0)
    {
        int chunkSize = juce::jmin(subBlockSize, samplesRemaining);

        // Samples outer, channels inner — shared SmoothedValues advance
        // once per sample so both channels receive identical values.
        for (int i = 0; i < chunkSize; ++i)
        {
            float drive  = driveSmoothed.getNextValue();
            float mixVal = mixSmoothed.getNextValue() / 100.0f;
            float vol    = volumeSmoothed.getNextValue();

            for (int ch = 0; ch < numChannels; ++ch)
            {
                float* channelData = buffer.getWritePointer(ch);
                int idx = startSample + i;

                float dry = channelData[idx];
                float val = dry * drive;
                float wet = (2.f / juce::float_Pi) * std::atan(val);

                channelData[idx] = ((1.0f - mixVal) * dry + mixVal * wet) * vol;
            }
        }

        startSample += chunkSize;
        samplesRemaining -= chunkSize;
    }
}

//==============================================================================
bool PluginTemplateAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginTemplateAudioProcessor::createEditor()
{
    return new PluginTemplateAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void PluginTemplateAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    // Create a temporary ValueTree object called copyState and assign it the value returned by apvts.copyState();
    juce::ValueTree copyState = apvts.copyState();

    // Create a unique_ptr to copy XML information
    std::unique_ptr<juce::XmlElement> xml = copyState.createXml();

    // Copy XML that we just created to our binary (Our Memory block)
    copyXmlToBinary(*xml.get(), destData);
}

void PluginTemplateAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    // Parse binary data back to XML — may be null if data is corrupt or empty
    std::unique_ptr<juce::XmlElement> xml = getXmlFromBinary(data, sizeInBytes);

    if (xml != nullptr)
    {
        juce::ValueTree copyState = juce::ValueTree::fromXml(*xml);
        if (copyState.isValid())
            apvts.replaceState(copyState);
    }
}

void PluginTemplateAudioProcessor::init()
{
    // Set SmoothedValue defaults to match parameter defaults
    driveSmoothed.setCurrentAndTargetValue(20.0f);    // DRIVE default = 20
    volumeSmoothed.setCurrentAndTargetValue(1.0f);    // VOL default = 0 dB = gain 1.0
    mixSmoothed.setCurrentAndTargetValue(0.0f);       // MIX default = 0%
}

void PluginTemplateAudioProcessor::prepare(double sampleRate, int samplesPerBlock)
{
    // Prepare DSP modules here (e.g., filters, convolution engines).
    // Cache sample rate if you need it for coefficient calculations:
    //   currentSampleRate = sampleRate;
    //
    // Set up a ProcessSpec for JUCE dsp modules:
    //   juce::dsp::ProcessSpec spec;
    //   spec.sampleRate = sampleRate;
    //   spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    //   spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());
}

void PluginTemplateAudioProcessor::update()
{
    mustUpdateProcessing.store(false, std::memory_order_relaxed);

    // Load parameter values via cached pointers (lock-free atomic reads)
    driveSmoothed.setTargetValue(driveParam->load());
    volumeSmoothed.setTargetValue(juce::Decibels::decibelsToGain(volParam->load()));
    mixSmoothed.setTargetValue(mixParam->load());
}

void PluginTemplateAudioProcessor::reset()
{
    // Set ramp times (50ms smoothing for all parameters)
    driveSmoothed.reset(getSampleRate(), 0.050);
    volumeSmoothed.reset(getSampleRate(), 0.050);
    mixSmoothed.reset(getSampleRate(), 0.050);
}

juce::AudioProcessorValueTreeState::ParameterLayout PluginTemplateAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    // Creates a function takes floats/ints and returns a string
    std::function<juce::String(float, int)> valueToTextFunction = [](float x, int l) { return juce::String(x, 4); };

    // Creates a function that takes a String and returns a float
    std::function<float(const juce::String&)> textToValueFunction = [](const juce::String& str) { return str.getFloatValue(); };

    // Add a Drive Parameter to our vector of parameters
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("DRIVE", "Drive", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 20.0f, "%", juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    // Add a Volume Parameter to our vector of parameters
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("VOL", "Volume", juce::NormalisableRange<float>(-40.0f, 40.0f), 0.0f, "db",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    // Add a Wet/Dry Parameter to our vector of parameters
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("MIX", "Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 0.0f, "%",
        juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));



    return { parameters.begin(), parameters.end() };
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginTemplateAudioProcessor();
}
