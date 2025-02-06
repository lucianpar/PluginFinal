#pragma once

#include <vector>
#include <cassert>
#include <cstdlib>

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

class History {
    float history = 0;
 public:

    float operator()(float f);
};

class DelayLine {
    std::vector<float> data;
    size_t next = 0;

 public:
    void resize(float seconds, float samplerate);
    void write(float f);
    float read(float delay);
    size_t size();
};

class KarplusStrong {
    DelayLine delayLine;
    History history;
    float beta = 0;
    float decay = 1;
    float delay = 0;

 public:
    void pluck(float hertz, float samplerate, float decay, float beta);
    float operator()();
};
class Timer {
    float value = 0;
    float increment = 0; // normalized frequency

 public:
    // called from the GUI
    void frequency(float hertz, float samplerate);

    // Called from processorBlock
    bool operator()();

};