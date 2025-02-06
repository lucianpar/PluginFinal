#include "PluginProcessor.h"
#include "PluginEditor.h"


juce::AudioProcessorValueTreeState::ParameterLayout parameters() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameter_list;

    parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
        "gain",
        "Gain",
        -60.0,
        0.0,
        -60.0));

    parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
        "frequency",
        "Frequency",
        0.0,
        127.0,
        60.0));

    parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
        "distortion",
        "Distortion",
        0.0,
        1.0,
        0.0));

    parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
        "rate",
        "Rate",
        0.0,
        1.0,
        0.0));

    return { parameter_list.begin(), parameter_list.end() };
}

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
     apvts(*this, nullptr, "Parameters", parameters())
{
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);

    Ramp ramp2;
    ramp2.frequency(440, static_cast<float>(getSampleRate()));
    int length = static_cast<int>(getSampleRate() * 2); // 2 second sound clip
    for (int i = 0; i < length; ++i) {
        float amp = 1 - 1.0f * i / length;
        amp = amp * amp * amp;
        player.addSample(amp * static_cast<float>(sin7(ramp2())));
    }

    timer.frequency(0.2, getSampleRate());
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

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

    float v = apvts.getParameter("gain")->getValue(); // (0, 1)
    float f = apvts.getParameter("frequency")->getValue();
    float t = apvts.getParameter("distortion")->getValue();
    float r = apvts.getParameter("rate")->getValue();

    auto mtof = [](float midi) {
        return 440 * pow(2, (midi - 69) / 12);
    };

    // (-f, 0). 0 is an amplitude of 1.
    auto dbtoa = [](float db) {
        return powf(10, db / 20);
    };

    //ramp.frequency(mtof(f * 127), getSampleRate());
    ramp.frequency(0.5, static_cast<float>(getSampleRate()));
    timer.frequency(7 * r, getSampleRate());
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        if (timer()) {
            ks.pluck(mtof(f * 127), getSampleRate(), 0.87, t);
        }
        /*
        float x = player(ramp()); // (-1, 1)
        float d = tanh(5 * f); // distorted version
        float mix = t * d + (1 - t) * x; // linear interpolation
        float sample = mix * dbtoa(-60 * (1 - v)); // get the next value of the ramp
        */
        float sample = ks() * dbtoa(-60 * (1 - v)); // get the next value of the ramp
        buffer.addSample(0, i, sample);
        buffer.addSample(1, i, sample);
    }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return false; //return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
