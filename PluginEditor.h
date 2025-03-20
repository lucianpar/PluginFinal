#pragma once

#include "PluginProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final
    : public juce::AudioProcessorEditor {
 public:
  explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
  ~AudioPluginAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics&) override;
  void resized() override;
  void openFile();

 private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  AudioPluginAudioProcessor& processorRef; // this is where i can change samplerate??? make upblic
  juce::Slider gainSlider;
  juce::Label  gainLabel;
  juce::Slider frequencySlider;
  //juce::Slider distortionSlider;
  juce::Slider rateSlider;

  juce::Slider delaySlider;
  juce::Label delayLabel;

  juce::Slider delaySlider2;
  juce::Label delayLabel2;

  juce::Slider grainLengthSlider;
  juce::Label grainLengthLabel;

  juce::Slider grainSpeedSlider;
  juce::Label grainSpeedLabel;

  juce::Slider birthRateSlider;
  juce::Label birthRateLabel;

  juce::Slider grainMixSlider;
  juce::Label grainMixLabel;
  std::vector<
      std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>>
      attachment;

  juce::TextButton openButton;
  std::unique_ptr<juce::FileChooser> chooser;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
