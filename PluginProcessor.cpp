#include "PluginProcessor.h"

#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout parameters() {
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameter_list;

  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParameterID{"gain", 1}, "Gain", -60.0, 0.0, -60.0));

  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParameterID{"frequency", 1}, "Frequency", 0.0, 127.0, 60.0));

  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParameterID{"rate", 1}, "Rate", 0.0, 1.0, 0.0));

  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParameterID{"delay", 1}, "Delay", 0.0, 2.0, 0.0));
  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParameterID{"delay2", 1}, "Delay2", 0.0, 2.0, 0.0));

  return {parameter_list.begin(), parameter_list.end()};
}

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
              ),
      apvts(*this, nullptr, "Parameters", parameters()) {
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool AudioPluginAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int AudioPluginAudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram() { return 0; }

void AudioPluginAudioProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return {};
}

void AudioPluginAudioProcessor::changeProgramName(int index,
                                                  const juce::String& newName) {
  juce::ignoreUnused(index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate,
                                              int samplesPerBlock) {
  // Use this method as the place to do any pre-playback
  // initialisation that you need..
  juce::ignoreUnused(sampleRate, samplesPerBlock);

  ky::setPlaybackRate(static_cast<float>(getSampleRate()));

  reverb.configure();

  // mDelayLine.setMaximumDelayInSamples(static_cast<int>(sampleRate));
  size_t maxDelaySamples = static_cast<size_t>(2.0 * sampleRate);
  delayLine.resize(maxDelaySamples);
  delayLine2.resize(maxDelaySamples);
  


  juce::dsp::ProcessSpec spec;
  spec.sampleRate = sampleRate;
  spec.maximumBlockSize = static_cast<uint32>(samplesPerBlock);
  spec.numChannels = static_cast<uint32>(getTotalNumOutputChannels());
  //convolution.prepare(spec);
}

void AudioPluginAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                             juce::MidiBuffer& midiMessages) {
  juce::ignoreUnused(midiMessages);

  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());
    
  
float v = apvts.getParameter("gain")->getValue();  // (0, 1)
float f = apvts.getParameter("frequency")->getValue();
// float t = apvts.getParameter("distortion")->getValue();
float r = apvts.getParameter("rate")->getValue();
float d = apvts.getParameter("delay")->getValue();
float d2 = apvts.getParameter("delay2")->getValue();

  

  // auto thing = this->buffer.exchange(nullptr, std::memory_order_acq_rel);

  ramp.frequency(ky::mtof(f * 127));
  timer.frequency(7 * r);

  float delaySamples = d * (0.5f * 44100);  // Normalize to 0 - 0.5 sec
  float delaySamples2 = d2 * (0.5f * 44100);

  delaySamples = std::min(delaySamples, static_cast<float>(delayLine.size() - 1)); //avoid out of bounds memory access
  delaySamples2 = std::min(delaySamples, static_cast<float>(delayLine.size() - 1));
  //ramp.frequency();
  for (int i = 0; i < buffer.getNumSamples(); ++i) {
    if (timer()) {
      env.set(0.05f, 0.3f);
    }
    float sample = env() * ky::sin7(ramp()) * ky::dbtoa(-60 * (1 - v));
    sample = reverb(sample);

    // float sample = player ? player->operator()(ramp()) : 0;

    //this section is where karl adds info to buffer  
     // Ensure delaySamples is within the buffer size
    // float delaySamples = d * (0.5f * 44100);  // Normalize to 0 - 0.5 sec
    // float delaySamples2 = d2 * (0.5f * 44100);

    // delaySamples = std::min(delaySamples, static_cast<float>(delayLine.size() - 1)); // Avoid out-of-bounds access

    float delayedSample = delayLine.read(delaySamples);  // Read from delay line
    float delayedSample2 = delayLine2.read(delaySamples2);
    delayLine.write(sample);  // Write current sample into delay line
    delayLine2.write(sample);

    // Mix Dry & Wet Signal 
    // float outputSample = (0.5f * sample) + (0.25f * delayedSample)+(0.25f * delayedSample2);
    float leftOutput = (0.5f * sample) + (0.5f * delayedSample);
    float rightOutput = (0.5f * sample) + (0.5f * delayedSample2);

    buffer.addSample(0, i, leftOutput);
    buffer.addSample(1, i, rightOutput);
  }

  juce::dsp::AudioBlock<float> block(buffer);
  // convolution.process(juce::dsp::ProcessContextReplacing<float>(block));
}

void AudioPluginAudioProcessor::setBuffer(
    std::unique_ptr<juce::AudioBuffer<float>> buffer) {
  juce::ignoreUnused(buffer);

  auto b = std::make_unique<ky::ClipPlayer>();
  for (int i = 0; i < buffer->getNumSamples(); ++i) {
    if (buffer->getNumChannels() == 2) {
      b->addSample(buffer->getSample(0, i) / 2 + buffer->getSample(1, i) / 2);
    } else {
      b->addSample(buffer->getSample(0, i));
    }
  }

  //player = std::move(b);
}

bool AudioPluginAudioProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor() {
  return new AudioPluginAudioProcessorEditor(*this);
}

void AudioPluginAudioProcessor::getStateInformation(
    juce::MemoryBlock& destData) {
  auto state = apvts.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void* data,
                                                    int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));

  if (xmlState.get() != nullptr)
    if (xmlState->hasTagName(apvts.state.getType()))
      apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new AudioPluginAudioProcessor();
}
