#include <JuceHeader.h>

#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <path_to_wav_file>" << std::endl;
        return 1;
    }

    juce::dsp::Convolution convolution;

    juce::File file(argv[1]);
    if (!file.exists()) {
        std::cout << "File does not exist" << std::endl;
        return 1;
    }

    convolution.loadImpulseResponse(file, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::no, 0);

    convolution.reset();

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = 48000;
    spec.maximumBlockSize = 1024;
    spec.numChannels = 2;
    convolution.prepare(spec);

    juce::AudioBuffer<float> buffer(1, 200000);
    buffer.clear();
    buffer.setSample(0, 0, 1.0f);

    juce::dsp::AudioBlock<float> block(buffer);
    convolution.process(juce::dsp::ProcessContextReplacing<float>(block));
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        std::cout << buffer.getSample(0, i) << std::endl;
    }
}
