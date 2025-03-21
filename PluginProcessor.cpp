#include "PluginProcessor.h"

#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout parameters() {
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameter_list;

  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParameterID{"gain", 1}, "Gain", -60.0, 0.0, -60.0));

  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParameterID{"delay", 1}, "Delay", 0.0, 4.0, 0.1));
  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParameterID{"delay2", 1}, "Delay2", 0.0, 4.0, 0.1));
  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParameterID{"grainLength", 1}, "grainLength", 0.1, 3.0, 0.5));
  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParameterID{"grainSpeed", 1}, "grainSpeed", -3.0, 3.0, 0.0));
  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParameterID{"birthRate", 1}, "birthRate", 0.0, 10.0, 0.1));
  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParameterID{"grainMix", 1}, "grainMix", 0.0, 1.0, 0.0));
  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParameterID{"grainPanLeft", 1}, "grainPanLeft", -1.0, 1.0,-0.5));
  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParameterID{"grainPanRight", 1}, "grainPanRight", -1.0, 1.0, 0.5));

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
      apvts(*this, nullptr, "Parameters", parameters())
      //, panWander(true, -1.0f, 1.0f, 0.0f)
       {
       // 
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

  //reverb.configure();

  // mDelayLine.setMaximumDelayInSamples(static_cast<int>(sampleRate));
  size_t maxDelaySamples = static_cast<size_t>(5.0 * sampleRate);
  delayLine.resize(maxDelaySamples);
  delayLine2.resize(maxDelaySamples);

  smoothedGrainMix.reset(sampleRate, 0.02); // ~20ms smoothing


  // size_t bufferSize = static_cast<size_t>(1 * sampleRate);  // 0.3 seconds of buffer
  // slicer.initialize(bufferSize);  // ✅ Initialize GranularSlicer with buffer size

  smoothedDelay.reset(sampleRate, 0.05);  // 50ms smoothing
  smoothedDelay2.reset(sampleRate, 0.05);

  currentSampleRate = sampleRate;

  // float panMin = -1.0f;  // Example min value
  // float panMax = 0.0f;   // Example max value

  //enable wandering 
  // float initialPanValue = apvts.getParameter("grainPanLeft")->getValue(); 
  // panWander.init(initialPanValue); 
  




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

    //param variables
    float v = apvts.getParameter("gain")->getValue();
    //float d = apvts.getParameter("delay")->getValue();
    //float d2 = apvts.getParameter("delay2")->getValue();
    //float gLen = apvts.getParameter("grainLength")->getValue();
    //float gSpeed = apvts.getParameter("grainSpeed")->getValue();
    //float bRate = apvts.getParameter("birthRate")->getValue();
    //float gMix = apvts.getParameter("grainMix")->getValue();


    //panning 
    float gPanL = *apvts.getRawParameterValue("grainPanLeft");
    float gPanR = *apvts.getRawParameterValue("grainPanRight");
    float d = *apvts.getRawParameterValue("delay");
    float d2 = *apvts.getRawParameterValue("delay2");
    float gLen = *apvts.getRawParameterValue("grainLength");
    float gSpeed = *apvts.getRawParameterValue("grainSpeed");
    float bRate = *apvts.getRawParameterValue("birthRate");
    float gMix = *apvts.getRawParameterValue("grainMix");

    smoothedGrainMix.setTargetValue(*apvts.getRawParameterValue("grainMix"));




    panWander.update();
    panWander2.update();
    //float wanderingPan = panWander.getValue();

    float finalPanLeft = juce::jlimit(-1.0f, 1.0f, gPanL + panWander.getValue() ); // Clamp in range - left grain pan slider value+wander increment
    float finalPanRight = juce::jlimit(-1.0f, 1.0f, gPanR + panWander2.getValue());
    

    float leftGrainPan = std::cos((finalPanLeft + 1.0f) * juce::MathConstants<float>::pi * 0.25f);
    float rightGrainPan = std::sin((finalPanRight + 1.0f) * juce::MathConstants<float>::pi * 0.25f);
  


    smoothedDelay.setTargetValue(d * currentSampleRate);
    smoothedDelay2.setTargetValue(d2 * currentSampleRate);

   //trigger for grains
    trigger.frequency(bRate); //nornmalize to 20hz

    //ramp for clip player
    ramp.frequency(0.1f);

    DBG(gMix);

    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        if (timer()) {
            env.set(0.05f, 0.3f);
        }

      
        //from clip player

        float sample = player ? player->operator()(ramp()) : 0;



        //when trigger happens, add a grain at that buffer position
        if (trigger()) {
          if (intermittency() > 0.8) break; //not currently using
          granulator.add(where(), (gLen), gSpeed);
        }
    
        float source = sample;
        float x = granulator();
        delayLine.write(x);
        delayLine2.write(x);

        //more smoothing
        float delayedSample = delayLine.read(smoothedDelay.getNextValue());
        float delayedSample2 = delayLine2.read(smoothedDelay2.getNextValue());

        float grainDelay1 = delayedSample * leftGrainPan;
        float grainDelay2 = delayedSample2 * rightGrainPan;

        float mix = smoothedGrainMix.getNextValue();

        float outL = ky::dbtoa(-60 * (1 - v)) *  ((source * (1.0-mix)) + (grainDelay1 * (mix)));
        float outR = ky::dbtoa(-60 * (1 - v))*  ((source * (1.0-mix)) + (grainDelay2 * (mix)));
        //float out = sample;
        buffer.addSample(0, i, outL);
        buffer.addSample(1, i, outR);
    }
    //juce::dsp::AudioBlock<float> block(buffer);
}








 //juce::dsp::AudioBlock<float> block(buffer);
  // convolution.process(juce::dsp::ProcessContextReplacing<float>(block));


void AudioPluginAudioProcessor::setBuffer(
    std::unique_ptr<juce::AudioBuffer<float>> buffer) {

  auto b = std::make_unique<ky::ClipPlayer>();
  for (int i = 0; i < buffer->getNumSamples(); ++i) {
    granulator.buffer.push_back(buffer->getSample(0, i));
    if (buffer->getNumChannels() == 2) {
       b->addSample(buffer->getSample(0, i) / 2 + buffer->getSample(1, i) / 2);
     } else {
       b->addSample(buffer->getSample(0, i));
     }
  }
  player = std::move(b);
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