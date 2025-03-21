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
  delaySlider.setRange(0, 4.0f, 0.05f);
  attachment.push_back(
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "delay", delaySlider));
  
  delaySlider2.setRange(0, 4.0f, 0.05f);
  attachment.push_back(
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "delay2", delaySlider2));

  grainLengthSlider.setRange(0.1f, 3.0f, 0.05f);
  attachment.push_back(
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "grainLength", grainLengthSlider));

  grainSpeedSlider.setRange(-3.0f, 3.0f, 0.1f);
  attachment.push_back(
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "grainSpeed", grainSpeedSlider));
  birthRateSlider.setRange(0.001f, 20.0f, 0.3f);
  attachment.push_back(
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "birthRate", birthRateSlider));
  grainMixSlider.setRange(0.0, 100.0, 1);
  attachment.push_back(
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "grainMix", grainMixSlider));

  grainPanLeftSlider.setRange(-1.0f, 1.0f, 0.1f);
  attachment.push_back(
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "grainPanLeft", grainPanLeftSlider));
  grainPanRightSlider.setRange(-1.0f, 1.0f, 0.1f);
  attachment.push_back(
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "grainPanRight", grainPanRightSlider));

  gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.attachToComponent(&gainSlider, false);

    delayLabel.setText("Delay", juce::dontSendNotification);
    delayLabel.attachToComponent(&delaySlider, false);

    delayLabel2.setText("Delay2", juce::dontSendNotification);
    delayLabel2.attachToComponent(&delaySlider2, false);

    grainLengthLabel.setText("Grain Length", juce::dontSendNotification);
    grainLengthLabel.attachToComponent(&grainLengthSlider, false);

    grainSpeedLabel.setText("Grain Speed", juce::dontSendNotification);
    grainSpeedLabel.attachToComponent(&grainSpeedSlider, false);

    birthRateLabel.setText("Birth Rate", juce::dontSendNotification);
    birthRateLabel.attachToComponent(&birthRateSlider, false);

    grainMixLabel.setText("Grain Mix", juce::dontSendNotification);
    grainMixLabel.attachToComponent(&grainMixSlider, false);

    grainPanLeftLabel.setText("Left Grains Pan", juce::dontSendNotification);
    grainPanLeftLabel.attachToComponent(&grainPanLeftSlider, false);

    grainPanRightLabel.setText("Right Grains Pan", juce::dontSendNotification);
    grainPanRightLabel.attachToComponent(&grainPanRightSlider, false);
    
  

  addAndMakeVisible(gainSlider);
  addAndMakeVisible(gainLabel);
  
  addAndMakeVisible(delaySlider);
  addAndMakeVisible(delayLabel);

  addAndMakeVisible(delaySlider2);
  addAndMakeVisible(delayLabel2);

  addAndMakeVisible(grainLengthSlider);
  addAndMakeVisible(grainLengthLabel);

  addAndMakeVisible(grainSpeedSlider);
  addAndMakeVisible(grainSpeedLabel);

  addAndMakeVisible(birthRateSlider);
  addAndMakeVisible(birthRateLabel);

  addAndMakeVisible(grainMixSlider);
  addAndMakeVisible(grainMixLabel);

  addAndMakeVisible(grainPanLeftSlider);
  addAndMakeVisible(grainPanLeftLabel);

  addAndMakeVisible(grainPanRightSlider);
  addAndMakeVisible(grainPanRightLabel);
  


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

    int buttonWidth = 150;  
    int buttonHeight = 20;  
    int topMargin = 20;  // Moves all elements down

    // Open Button Positioning
    openButton.setBounds((area.getWidth() - buttonWidth) / 2, 10, buttonWidth, buttonHeight);

    // Move content down
    area.removeFromTop(topMargin);

    // Define label height and spacing
    int labelHeight = 20;
    int padding = 5;  // Space between label & slider

    // Gain
    gainLabel.setBounds(area.removeFromTop(labelHeight));
    gainSlider.setBounds(area.removeFromTop(height - labelHeight - padding));

    // Delay
    delayLabel.setBounds(area.removeFromTop(labelHeight));
    delaySlider.setBounds(area.removeFromTop(height - labelHeight - padding));

    // Delay2
    delayLabel2.setBounds(area.removeFromTop(labelHeight));
    delaySlider2.setBounds(area.removeFromTop(height - labelHeight - padding));

    // Grain Length
    grainLengthLabel.setBounds(area.removeFromTop(labelHeight));
    grainLengthSlider.setBounds(area.removeFromTop(height - labelHeight - padding));

    // Grain Speed
    grainSpeedLabel.setBounds(area.removeFromTop(labelHeight));
    grainSpeedSlider.setBounds(area.removeFromTop(height - labelHeight - padding));

    // Birth Rate
    birthRateLabel.setBounds(area.removeFromTop(labelHeight));
    birthRateSlider.setBounds(area.removeFromTop(height - labelHeight - padding));

    grainMixLabel.setBounds(area.removeFromTop(labelHeight));
    grainMixSlider.setBounds(area.removeFromTop(height - labelHeight - padding));

    grainPanLeftLabel.setBounds(area.removeFromTop(labelHeight));
    grainPanLeftSlider.setBounds(area.removeFromTop(height - labelHeight - padding));

    grainPanRightLabel.setBounds(area.removeFromTop(labelHeight));
    grainPanRightSlider.setBounds(area.removeFromTop(height - labelHeight - padding));
}

