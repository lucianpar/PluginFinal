#include "PluginEditor.h"

#include "PluginProcessor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p) {
  setSize(500, 400);

  attachment.push_back(
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "gain", gainSlider));
  attachment.push_back(
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "frequency", frequencySlider));
  // attachment.push_back(
  //     std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
  //         processorRef.apvts, "distortion", distortionSlider));
  attachment.push_back(
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "rate", rateSlider));
  attachment.push_back(
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "delay", delaySlider));
  attachment.push_back(
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "delay2", delaySlider2));

  addAndMakeVisible(gainSlider);
  addAndMakeVisible(frequencySlider);
  //addAndMakeVisible(distortionSlider);
  addAndMakeVisible(rateSlider);
  addAndMakeVisible(delaySlider);
  addAndMakeVisible(delaySlider2);

  chooser = std::make_unique<juce::FileChooser>(
      "Select a file to open...",
      juce::File::getSpecialLocation(juce::File::userDesktopDirectory),
      "*.wav;*.mp3;*.aif;*.flac");
  openButton.setButtonText("select audio file");
  openButton.onClick = [this] {
    auto fn = [this](const juce::FileChooser& c) {
      juce::File file = c.getResult();
      if (!file.exists()) return;

      juce::AudioFormatManager formatManager;
      formatManager.registerBasicFormats();
      std::unique_ptr<juce::AudioFormatReader> reader(
          formatManager.createReaderFor(file));
      if (reader == nullptr) return;

      auto buffer = std::make_unique<juce::AudioBuffer<float>>(
          static_cast<int>(reader->numChannels),
          static_cast<int>(reader->lengthInSamples));

      reader->read(buffer.get(), 0, static_cast<int>(reader->lengthInSamples),
                   0, true, true);

      // auto old = processorRef.buffer.exchange(buffer.release(),
      // std::memory_order_acq_rel); if (old) delete old;
      processorRef.setBuffer(std::move(buffer));
    };

    chooser->launchAsync(juce::FileBrowserComponent::canSelectFiles, fn);
  };

  addAndMakeVisible(openButton);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void AudioPluginAudioProcessorEditor::resized() {
  auto area = getLocalBounds();
  auto height = 40;
  //openButton.setBounds(area.removeFromTop(height));
  gainSlider.setBounds(area.removeFromTop(height));
  frequencySlider.setBounds(area.removeFromTop(height));
  //distortionSlider.setBounds(area.removeFromTop(height));
  rateSlider.setBounds(area.removeFromTop(height));
  delaySlider.setBounds(area.removeFromTop(height));
  delaySlider2.setBounds(area.removeFromTop(height));
}
