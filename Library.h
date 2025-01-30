#pragma once

#include <vector>

// expects x on (0, 1)
double sin7(double x);

class Ramp {
    float value = 0;
    float increment = 0; // normalized frequency

public:
    // called from the GUI
    void frequency(float hertz, float samplerate);

    // Called from processorBlock
    float operator()();

};

class ClipPlayer {
    std::vector<float> data; // push_back()

public:
    void addSample(float sample);

    // expects phase on [0, 1) 
    float operator()(float phase);
};
