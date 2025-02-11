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

class CombFeedback {
    DelayLine delayLine;
    float delayTime = 0;
    float feedback = 0;
    public:

    void configure(float delaySeconds, float samplerate, float gain) {
        feedback = gain;
        delayTime = delaySeconds * samplerate;
        delayLine.resize(delaySeconds, samplerate);
    }

    float operator()(float input) {
        float output = input + feedback * delayLine.read(delayTime);
        delayLine.write(output);
        return output;
    }
};

class AllPass {
    DelayLine delayLine;
    float gain = 0;
    float delay = 0;

    public:

    void configure(float delaySeconds, float samplerate, float gain) {
        delayLine.resize(delaySeconds, samplerate);
        delay = delaySeconds * samplerate;
        this->gain = gain;
    }

    float operator()(float input) {
        float read = delayLine.read(delay);
        float vn = input - gain * read;
        delayLine.write(vn);
        float output = read + vn * gain;
        return output;
    }
};

class ShroederReverb {
    std::vector<CombFeedback> comb;
    std::vector<AllPass> allpass;
    public:

    void configure(float samplerate) {
        comb.resize(4);
        comb[0].configure(1687.0 / samplerate, samplerate, 0.773);
        comb[1].configure(1601.0 / samplerate, samplerate, 0.802);
        comb[2].configure(2053.0 / samplerate, samplerate, 0.753);
        comb[3].configure(2251.0 / samplerate, samplerate, 0.733);

        allpass.resize(3);
        allpass[0].configure(347.0 / samplerate, samplerate, 0.7);
        allpass[1].configure(113.0 / samplerate, samplerate, 0.7);
        allpass[2].configure(37.0 / samplerate, samplerate, 0.7);
    }

    float operator()(float input) {
        float output = 0;

        for (auto& filter : comb) {
            output += filter(input);
        }

        for (auto& filter : allpass) {
            output = filter(output);
        }

        return output;
    }
};

class DCblock {
    float x1 = 0;
    float y1 = 0;
    public:

    float operator()(float input) {
        float output = input - x1 + 0.995 * y1;
        y1 = output;
        x1 = input;
        return output;
    }
};