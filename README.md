# JUCE-Plugin-Template
The perfect starting point to build a VST plugin

# Basic Juce Setup for Windows

1. Install Visual Studio Community: https://visualstudio.microsoft.com/vs/community/
2. Install GIT: https://git-scm.com/download/win
3. Install CMAKE: https://cmake.org/download/ (choose Windows x64 Installer)
4. Open the command prompt
5. [Windows key + R] to open the run dialog, then type “cmd.exe” and click OK. You can also go to your C:\ folder and use [Alt + F], and click "Open Windows Powershell" to open up the powershell prompt in the C:\ folder. 
6. Navigate to root directory with the command:
```BASH
cd C:\
```
7. Clone the JUCE repository:
```BASH
git clone https://github.com/juce-framework/JUCE.git
```
8. Navigate to the JUCE directory:
```BASH
cd C:\JUCE\
```
9. Build the Projucer app:
```cmake
cmake . -B cmake-build -DJUCE_BUILD_EXAMPLES=ON -DJUCE_BUILD_EXTRAS=ON
cmake --build cmake-build --target Projucer
```
10. Open Windows Explorer and navigate to the directory:
```
C:\JUCE\cmake-build\extras\Projucer\Projucer_artefacts\Debug\
```
11. Right-click “Projucer.exe” select “Send to” -> “Desktop (create shortcut)”

You can now select the Projucer app from your desktop, load an example project, open it in Visual
Studio, and select debug to run on your machine.

# Tutorial: Create a template for any plugin and use the template to create a simple Distortion effect plug-in
- by Ryan Laur

The following template was developed with the help of Ivan Cohen and The Audio Programmer, AKA Joshua Hodge. Ivan has developed a number of commercial plugins over the years, and the Audio Programmer is a Audio Programming Educator that has served as a mentor to AES at the University of Florida.

https://github.com/musicalentropy

https://github.com/TheAudioProgrammer
https://www.theaudioprogrammer.com/development

This tutorial will serve to teach some of the basic concepts to set up any audio plugin for processing. 

## Creating a Project with Projucer

First, you will open Projucer, and select your plugin type as Basic, for a VST plugin, and the plugin name:

![image](https://user-images.githubusercontent.com/56581520/199033731-87469e2c-ecc1-4b32-94ad-32ccaf4fb308.png)

Next, make sure you select any extra modules you will need, (like dsp). For now, we will skip this step and move on to the compiler. We need to add the compiler we will be using, in this case we are using Visual Studio 2019:

![image](https://user-images.githubusercontent.com/56581520/199033333-9517b1c5-ee03-4e7d-af53-15ccf09d59f1.png)

Now you can select Create Project which will prompt you to save the project. 

### Projucer Options

Company Name, and Plugin Information:

![image](https://user-images.githubusercontent.com/56581520/199035586-f50b3b86-e49d-4f39-a85a-69bdbec48cdd.png)

*Your comapny name serves as an identifier, so always use a name you will recognize, even if you don't have a company.*

#### Enable Plugin Copy Step

Next you can click debug under exporters, and scroll down to the enable copy plugin option. Here you can enable copying the VST to your VST folder for your DAW.

![image](https://user-images.githubusercontent.com/56581520/199035212-8695dd26-974e-42ff-be1e-eb36bea269ff.png)

**Note: You will need to set write permissions for this folder (for normal users). Otherwise, in order to complete the plugin copy step, Visual Studio will need to run in administrator**

For plugin Characteristics, our plugin is not a midi plugin, so we will skip that step. 
## Essential Components of plug-in DSP Processing
![image](https://user-images.githubusercontent.com/56581520/195697102-58f22ff7-3719-4cd2-b03a-5b270d68229b.png)

## Getting Started

### Quick Review

A newly-created audio plug-in project contains two main classes: 
* PluginProcessor handles the audio and MIDI IO and processing logic
* PluginEditor handles any on screen GUI controls or visualisations.

## Creating Template Functions

First, go to the PluginProcessor.h (header file) where we will start to add our first function prototypes.

In C++, member functions usually go in the public section of the class. 

Navigate to the bottom of the class and add the following code:

```C++
    //==============================================================================
    
    // Give DSP initial values
    void init();
    
    // Pass sample rate and buffer size to DSP 
    void prepare (double sampleRate, int samplesPerBlock);
    
    // Called when user changes parameters
    void update();
    
    // Reset DSP parameters
    void reset() override;
```
*Here we call the function prepare() so it doesn't interfere with preparetoplay.*

**For the reset(), we use the override keyword because as shown in the JUCE documentation:**
![image](https://user-images.githubusercontent.com/56581520/195703799-7f0c20db-cd75-494a-be32-ac6e0428f0c7.png)

### Easily Create Function Definitions
**You can follow these steps to easily create the definitions for these prototypes:**
1. Highlight the function prototypes
2. Right-Click and select Quick Actions and Refactorings

![image](https://user-images.githubusercontent.com/56581520/195704755-e2c13478-20d3-4d5c-aedb-842b81768a67.png)

3. Select Create Declaration / Defintion

![image](https://user-images.githubusercontent.com/56581520/195705239-980067a5-225f-40fe-a786-0f957a543f87.png)

### Calling Our Functions

Now, we will go to our PluginProcessor.cpp file and look for the following code:

```C++
//==============================================================================
PluginTemplateAudioProcessor::PluginTemplateAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}
```
*This is the Default Constructor for the AudioProcessor Class.*
Inside the Braces { }, add our init() function:

```C++
{
    init();
}

```

Next we need to go to preparetoplay():

![image](https://user-images.githubusercontent.com/56581520/195707983-fb942b2e-c583-4bdd-915f-939ed776684b.png)

Add the following code:
```C++
prepare (sampleRate, samplesPerBlock);
update(); // pass information to our Algorithm
reset(); // Clear Junk Values, etc.
```

We would also like to call the update() function within the process block, but only when a user has changed a parameter. So, let's set up a ValueTree Listener to *listen* for these changes.

### ValueTree Listener Class

**Looking at the Juce documentation, we can find the implementation of the class:**

![image](https://user-images.githubusercontent.com/56581520/195711457-3518647f-8da7-42d7-81fd-7ba5b1c9ae16.png)


Head to the top of the Audio Processor header file, and add the following code marked with a **[!!!]**:
```C++
class PluginTemplateAudioProcessor  : public AudioProcessor,
                                      public juce::ValueTree::Listener // [!!!]
{
public:
    //==============================================================================
    PluginTemplateAudioProcessor();
    ~PluginTemplateAudioProcessor();
```
*This code allows our AudioProcessor to inherit member functions an capabilities from the ValueTree::Listener class*

Head over to the processor header file and add the following code to the private section:

```C++
private:
    bool isActive { false };
    bool mustUpdateProcessing { false };
    // Called when user changes a parameter
    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property) override
    {
        mustUpdateProcessing = true;
    }
```

* **isActive** will be used to tell our processor when the plug-in is active.
* **mustUpdateProcessing** will be used to tell our processor when to update the parameters our processor uses. 

Finally, add the Following to our Process Block (At the top) in AudioProcessor.cpp
```C++
    if (!isActive)
        return;
    
    if (mustUpdateProcessing)
        update();
```
*This function will be called when a property in our valueTree has changed.*

Now we need to set isActive to true inside prepareToPlay function:
```C++
    isActive = true;
```

### AudioProcessorValueTreeState

Now we need to create an object for our Value Tree State, and a function to create parameters for the Tree.

**How we are going to add our parameters:**

![image](https://user-images.githubusercontent.com/56581520/195715159-d509ebf8-764b-432e-8a65-f8e0b607531a.png)


Add the following to the public section of the AudioProcessor.h header file:
```C++
    // Store Parameters
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
```

#### Building

**Wait to Build until you create add a listener, call createParameters() in the Audio Processor Default Constructor, and the definition for the createParameters() function, as shown below.**

### Initialize and create parameters
Once this is added, we actually have to initialize the object and call our createParameters() function.

We will do this in the default constructor in the AudioProcessor.cpp file, in the initialization section.

*To understand how to initialize the AudioProcessorValueTreeState Object, we can right-click on the class name, and click go to definition.*

**There, we can find the Default Constructor for the class that initializes our parameter layout:**

![image](https://user-images.githubusercontent.com/56581520/195714025-b9330920-b001-4eb5-9b95-2e360b3f6ade.png)

Next, we will add a *listener* the the object:
```C++
//==============================================================================
PluginTemplateAudioProcessor::PluginTemplateAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts (*this, nullptr, "Parameters", createParameters()) // [1] Add this code!
#endif
{
    apvts.state.addListener (this); // [2] Add this too!
    init();
}
```

### createParameters() Function

Using the same method as before, use Quick Actions to create a definition for the createParameters() function, which will appear in PluginProcessor.cpp.

Add the following code:
```C++
juce::AudioProcessorValueTreeState::ParameterLayout PluginTemplateAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    return { parameters.begin(), parameters.end() };
}

```
*Creates a vector of unique_ptr (unique pointers) to hold our parameters, and returns the vector of parameters.*

## Saving and Restoring Plug-in Parameters

We are going to create functionality for if we want to save the changes that a user has made to our plug-in, we can now save that using the getStateInformation() function, and have the ability to store that information using the setStateInformation. 

### Accessing the Changed State Information

Navigate to the getStateInformation() function in AudioProcessor.cpp:

![image](https://user-images.githubusercontent.com/56581520/196063696-5a6f1b26-c3fb-4939-b472-08da5c241fe6.png)

*This is where we can get the information the user has changed about the plugin. In C++ this is known as an **Accessor Function***

We will need some XML information which can be seen here:

![image](https://user-images.githubusercontent.com/56581520/196064113-95dc76da-fc30-41bb-a1e9-0ce6167f9634.png)

**JUCE Documentation**

![image](https://user-images.githubusercontent.com/56581520/196064227-8af14dc0-3491-48bd-8458-86cc6cfe7d20.png)

**Hover to view the inputs to the function**

Next, add the following code:

```C++
// Create a temporary ValueTree object called copyState and assign it the value returned by apvts.copyState();
juce::ValueTree copyState = apvts.copyState();

// Create a unique_ptr to copy XML information
std::unique_ptr<juce::XmlElement> xml = copyState.createXml();

// Copy XML that we just created to our binary (Our Memory block)
copyXmlToBinary(*xml.get(), destData);
```

**Note: Why don't we create the XML on the AudioProcessorValueTreeState object directly using apvts.createXml()? The reason for this has to do with thread-switching - essentially, the copyState() function allows you to safely copy values over and there may be a case where the thread would switch in the middle of a copy.**

### Storing the Changed State Information

Navigate to the setStateInformation() function in AudioProcessor.cpp:

![image](https://user-images.githubusercontent.com/56581520/196063663-1eb74100-5de3-422d-be86-4f98ed540e2a.png)

Add the following code:

```C++
// Create an xml pointer, and get the XML from the binary (our memory block)
std::unique_ptr<juce::XmlElement> xml = getXmlFromBinary(data, sizeInBytes);

// Create a temporary ValueTree called copyState and save that data into our ValueTree object
juce::ValueTree copyState = juce::ValueTree::fromXml(*xml.get());

// Now we will replace the state with our copyState object in our apvts object
apvts.replaceState(copyState);
```

**Now we are able to save our parameter changes and store them at a later point!**

## Creating a Gain Parameter

Now we are going to demonstrate how to add a parameter to your ValueTree. Previously, we created a function called createParameters() that returns a vector of our unique pointers of our Ranged Audio Parameters. 

![image](https://user-images.githubusercontent.com/56581520/196064826-90f57623-2a3d-4a8d-82d1-68d9c2a51f6c.png)

**We see the Ranged Audio Parameter where we will use the AudioParameterFloat for decimal numbers**

![image](https://user-images.githubusercontent.com/56581520/196064896-606e4afd-584d-4ede-a8fd-1fc5e6607c61.png)

**Within the AudioParameterFloat Documentation, we can see that we have two different Constructors we can call. We are going to the longer version.**

First, navigate to the createParameters() function:

![image](https://user-images.githubusercontent.com/56581520/196064785-794c8662-38a0-4d4c-a683-a7ab421d6778.png)

Add the following code below our vector, and above the return:
```C++

// Creates a function takes floats/ints and returns a string
std::function<juce::String(float, int)> valueToTextFunction = [](float x, int l) { return juce::String(x, 4); };

// Creates a function that takes a String and returns a float
std::function<float (const juce::String&)> textToValueFunction = 
[](const juce::String& str) { return str.getFloatValue(); };

// Add a Volume Parameter to our vector of parameters
parameters.push_back (std::make_unique<juce::AudioParameterFloat>("DRIVE", "Drive", 
juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 20.0f, "%", 
juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));
```

## The Audio Callback (processBlock) and Creating Distortion Effect

This is where the audio is coming in and going out of your plug-in. This is where our digital signal processes/processing chain is going to be implemented - where the real-time processing happens AKA the "guts" of our plug-in. 

![image](https://user-images.githubusercontent.com/56581520/196066880-13416615-ffc8-48e0-8130-72feae32f993.png)

**The Audio Here is going between +1 and -1, where the middle line is 0.**

![image](https://user-images.githubusercontent.com/56581520/197617337-8cc94e6e-e063-4017-9a9b-119e3f934d78.png)

*This is the LinearSmoothedValue typename*

![image](https://user-images.githubusercontent.com/56581520/197617563-29f14b70-b8ee-49bc-a98e-33e72dfb116b.png)

*Part of the SmoothedValue Utility Class*

First, In PluginProcessor.h, in the private section, we need to add:
```C++
    juce::LinearSmoothedValue<float> driveNormal { 0.0 };
```
**This creates our Normalized Drive Parameter**

Head over to our processblock:

![image](https://user-images.githubusercontent.com/56581520/196067010-96ecb70c-783e-41e9-9368-0641e3fb4a7c.png)

**In the function parameters, the AudioBuffer (buffer) is a Vector of floats where we are going to put our Audio Information. First, we make sure the plugin is actually instantiated before we begin processing audio, and then we only want to update our DSP processes when we need to (don't need to do calculations every time)**

* ScopedNoDenormals - After a certain number of values for very small numbers, it will zero them out
* buffer.clear - Clears junk values in buffer before adding data
* totalNumInputChannels - One for each speaker, or each channel in stereo

In our main block, we iterate through each of the channels L/R, and then we use,
```C++
auto* channelData = buffer.getWritePointer (channel);
```
This gets the write pointer for the channel, into channelData. Next, remove the ignore function.

Now, add the following code:
```C++
        // Iterate over each sample in this channel
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {

                float val = channelData[sample] * driveNormal.getCurrentValue();
                channelData[sample] = (2.f / juce::float_Pi) * atan(val); //apply distortion
        }
```

### Loading our Parameter

Inside our update() function, we need to add the code to load the value from our slider into our Class driveNormal variable.
```C++
void Distortion_ProjectAudioProcessor::update()
{
    mustUpdateProcessing = false;

    //Load variables from APVTS
    auto drive = apvts.getRawParameterValue("DRIVE");
    driveNormal = drive->load();

```
**update() is the Accessor Function that allows use to change the member variable driveNormal**

Next we reset our drive variable in the reset() function.  
```C++
void PluginTemplate::AudioProcessor::reset()
{
    driveNormal.reset(getSampleRate(), 0.050);
```

### Generic Processor Editor

Say we want to test our plug-in quickly. Let's add this code to the createEditor() class, comment out the line there. It will now add generic  sliders to our plug-in. 
```C++
AudioProcessorEditor* PluginTemplateAudioProcessor::createEditor()
{
    //return new PluginTemplateAudioProcessorEditor (*this);
    return new GenericAudioProcessorEditor(*this);
}

```

![image](https://user-images.githubusercontent.com/56581520/197619212-f988cb73-2bfe-4f57-b38b-f4f24cf94e1a.png)

## Creating Volume, Wet, and Dry Parameters

To create a parameter, we first need to create an object: 

In AudioProcessor.h private Section:
```C++
    juce::LinearSmoothedValue<float> outputVolume[2]{ 0.0 };
    juce::LinearSmoothedValue<float> outputMix[2]{ 0.0 };
```

*We now add a LinearSmoothedValue parameter array of size 2, to use as a stereo parameter.*

Next, we must add the parameter to our vector of parameters: 

In createParameters():

```C++
    // Add a Drive Parameter to our vector of parameters
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("DRIVE", "Drive", 
juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 20.0f, "%", juce::AudioProcessorParameter::genericParameter, 
valueToTextFunction, textToValueFunction));

    // Add a Volume Parameter to our vector of parameters
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("VOL", "Volume", 
juce::NormalisableRange<float>(-40.0f, 40.0f), 0.0f, "db", 
juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

    // Add a Wet/Dry Parameter to our vector of parameters
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("MIX", "Mix", 
juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 0.0f, "%", 
juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));
```

*Here we added both Volume and Mix parameters*

Head over to reset(), where we will reset our new objects:

```C++
void PluginTemplateAudioProcessor::reset()
{
    driveNormal.reset(getSampleRate(), 0.050);

    for (int channel = 0; channel < 2; ++channel)
    {
        // reset(sampleRate, rampLength in seconds)
        outputVolume[channel].reset(getSampleRate(), 0.001);
        outputMix[channel].reset(getSampleRate(), 0.001);
    }
}
```

Because the volume is in dB, we need to convert the decibels to magnitude. This is shown below. We also set the target value, so we smooth the parameter appropriately. 
In update():
```C++
void PluginTemplateAudioProcessor::update()
{
    mustUpdateProcessing = false;

    //Load variables from APVTS
    auto drive = apvts.getRawParameterValue("DRIVE");
    auto volume = apvts.getRawParameterValue("VOL");
    auto mix = apvts.getRawParameterValue("MIX");

    driveNormal = drive->load();

    for (int channel = 0; channel < 2; ++channel)
    {
        outputVolume[channel].setTargetValue(juce::Decibels::decibelsToGain(volume->load()));
        outputMix[channel].setTargetValue(mix->load());
    }

}
```

In the Processblock():

```C++
void PluginTemplateAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    if (!isActive)
        return;

    if (mustUpdateProcessing)
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
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        // ..do something to the data...

        // Iterate over each sample in this channel
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            auto mixVal = outputMix[channel].getNextValue()/100.0f;
            float val = channelData[sample] * driveNormal.getCurrentValue();
            float wet = (2.f / juce::float_Pi) * atan(val); //apply distortion
            float dry = channelData[sample];

            channelData[sample] = (1.0f - mixVal) * dry + mixVal * wet;

        }

        outputVolume[channel].applyGain(channelData, numSamples);
    }
}
```

Now:
```C++
AudioProcessorEditor* PluginTemplateAudioProcessor::createEditor()
{
    return new PluginTemplateAudioProcessorEditor (*this);
    //return new GenericAudioProcessorEditor(*this);
}

```

Now head over to PluginEditor.h, where we will add our Slider objects as unique pointers. 

Add this to the bottom of the public section:

```C++
    // using Attachment to shorten the type
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
```

Add this above the audioProcessor object in the private section:

```C++
private:

    // Slider Objects
    std::unique_ptr<juce::Slider> driveSlider, volSlider, mixSlider;

    // Slider Label Objects
    std::unique_ptr<juce::Label> driveLabel, volLabel, mixLabel;

    // Attachments
    std::unique_ptr<Attachment> driveAttachment, volAttachment, mixAttachment;
```

Next, go to PluginEditor.cpp and add the following code:

```C++
PluginTemplateAudioProcessorEditor::PluginTemplateAudioProcessorEditor (PluginTemplateAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{

    // Volume Slider
    driveSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::RotaryVerticalDrag, 
juce::Slider::TextBoxBelow);
    addAndMakeVisible(driveSlider.get());

    // Volume Label
    driveLabel = std::make_unique<juce::Label>("", "Drive");
    addAndMakeVisible(driveLabel.get());
    driveLabel->attachToComponent(driveSlider.get(), false);
    driveLabel->setJustificationType(juce::Justification::centred);

    // Volume Slider
    volSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::RotaryVerticalDrag, 
juce::Slider::TextBoxBelow);
    addAndMakeVisible(volSlider.get());

    // Volume Label
    volLabel = std::make_unique<juce::Label>("", "Volume");
    addAndMakeVisible(volLabel.get());
    volLabel->attachToComponent(volSlider.get(), false);
    volLabel->setJustificationType(juce::Justification::centred);

    // mix Slider
    mixSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::RotaryVerticalDrag, 
juce::Slider::TextBoxBelow);
    addAndMakeVisible(mixSlider.get());

    // mix Label
    mixLabel = std::make_unique<juce::Label>("", "Wet/Dry");
    addAndMakeVisible(mixLabel.get());
    mixLabel->attachToComponent(mixSlider.get(), false);
    mixLabel->setJustificationType(juce::Justification::centred);

    // Attachments
    //using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    driveAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "DRIVE", *driveSlider);
    volAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "VOL", *volSlider);
    mixAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "MIX", *mixSlider);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 400);
}

PluginTemplateAudioProcessorEditor::~PluginTemplateAudioProcessorEditor()
{
}

//==============================================================================
void PluginTemplateAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto textBounds = bounds.removeFromTop(40);

    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.fillRect(bounds);

    g.setColour(juce::Colours::black);
    g.fillRect(textBounds);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(20.0f).italicised().withExtraKerningFactor(0.1f));
    g.drawFittedText("DSP Lesson 1", textBounds, juce::Justification::left, 1);
}

void PluginTemplateAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

        // bounds is the "background" of our plug-in
    auto bounds = getLocalBounds();

    // Give 40 pixels of space for the title
    auto textBounds = bounds.removeFromTop(40);

    // Reserve some room for the gain meter
    //bounds.removeFromRight(40);

    // Give 40 pixels of margin on all sides so we don't paint to the edges of the background
    bounds.reduce(40, 40);

    juce::Grid grid;

    // TrackInfo is a "space"
    using Track = juce::Grid::TrackInfo;

    // Fr is how big the space is relative to other spaces
    using Fr = juce::Grid::Fr;

    // 4 columns x 2 rows
    grid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
    grid.templateRows = { Track(Fr(1)), Track(Fr(1)) };

    // Margin between each space
    grid.rowGap = juce::Grid::Px(20);
    grid.columnGap = juce::Grid::Px(20);

    // Add components to the grid
    grid.items.add(driveSlider.get());
    grid.items.add(volSlider.get());
    grid.items.add(mixSlider.get());

    // Perform the layout on the bounds
    grid.performLayout(bounds);
}

```


