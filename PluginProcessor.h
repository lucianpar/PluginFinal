#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>

#include "Library.h"

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor {
 public:
  //==============================================================================
  
        
  AudioPluginAudioProcessor();
  ~AudioPluginAudioProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  using AudioProcessor::processBlock;

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
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String& newName) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  void setBuffer(std::unique_ptr<juce::AudioBuffer<float>> buffer);

  juce::AudioProcessorValueTreeState apvts;
  // std::atomic<juce::AudioBuffer<float>*> buffer;
  // juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> mDelayLine{44100};
  

 private:
 ky::Granulator granulator;
 ky::Granulator granulator2;
 ky::Timer trigger1;
 ky::Timer trigger2;
 ky::Noise asynchrony, intermittency, wobble, where;
  ky::Ramp ramp;
  ky::Timer timer;
  ky::SchroederReverb reverb, reverb2;
  ky::AttackDecay env;
  ky::DelayLine delayLine;
  ky::DelayLine delayLine2;
  float currentSampleRate;
  float smoothedSamplesAgo = 0.0f;
  float smoothedSamplesAgo2 = 0.0f;  // For second delay line
  // juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> mDelayLine{44100};

  juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedDelay;
  juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedDelay2;

  juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedGrainMix;
  juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedBirthRate;


  std::unique_ptr<ky::ClipPlayer> player;


   
  // juce::dsp::Convolution convolution;

  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
  //GranularSlicer slicer; 
  ky::Wander panWander{0.005f}; // step size of 0.005
  ky::Wander panWander2{0.008f};
};

juce::AudioProcessorValueTreeState::ParameterLayout parameters();
