#pragma once

#include <vector>
#include <cassert>
#include <cstdlib>

namespace ky {

inline float sin7(float x) {   
    // expects x on (0, 1)
    return x*(x*(x*(x*(x*(x*(194.27296f-55.50656f*x)-213.704224f)+48.57816f)+31.77344f)+0.89816f)-6.311936f);                                                                            
} 

inline float mtof(float midi) {
    return 440 * powf(2, (midi - 69) / 12);
}

inline float dbtoa(float db) {
    return powf(10, db / 20);
}

struct PlaybackRateObserver {
    float samplerate{1};
    PlaybackRateObserver* nextObserver{nullptr};
    virtual void onPlaybackRateChange(float samplerate);
    PlaybackRateObserver();
};

class PlaybackRateSubject {
    PlaybackRateObserver* list{nullptr};
    PlaybackRateSubject() {}

 public:
    PlaybackRateSubject(PlaybackRateSubject const&) = delete;
    void operator=(PlaybackRateSubject const&) = delete;

    void notifyObservers(float samplerate);
    void addObserver(PlaybackRateObserver* observer);
    static PlaybackRateSubject& instance();
};

// convenience function for setting playback rate
inline void setPlaybackRate(float samplerate) {
    PlaybackRateSubject::instance().notifyObservers(samplerate);
}

class Ramp : public PlaybackRateObserver {
    float value = 0;
    float increment = 0; // normalized frequency

 public:
    void frequency(float hertz) {
        assert(samplerate != 0.0f);
        increment = hertz / samplerate; 
    }

    float operator()() {
        float v = value;

        value += increment;

        if (value >= 1.0){
            value -= 1.0f;
        }

        return v;
    }
};

class STFT : public PlaybackRateObserver {
    public:

    bool operator()(float) {
        // when we have enough samples, take the FFT and return true; otherwise false
        return true;
    }

    float operator()() {
        // return the inverse FFT of the internal spectral data
        return 0;
    }
};


class ClipPlayer : public PlaybackRateObserver {
    std::vector<float> data; // push_back()

 public:
    void addSample(float f) {
        data.push_back(f);
    }

    // input on (0, 1)
    // linear interpolation
    float operator()(float phase) {
        float index = phase * data.size();
        unsigned a = static_cast<unsigned>(index);
        unsigned b = (a == (data.size() - 1)) ? 0 : a + 1;
        float t = index - a;
        return data[b] * t + (1 - t) * data[a];
    }
};

class History {
    float history = 0;
 public:

    float operator()(float f) {
        float v = history;
        history = f;
        return v;
    }
};

class DelayLine {
    std::vector<float> data;
    size_t next = 0;

 public:
    void resize(size_t size) {
        data.resize(size, 0);
        next = 0;
    }

    size_t size() { return data.size(); }

    void write(float f) {
        assert(!data.empty());
        data[next] = f;
        next = (1 + next) % data.size();
    }

    // delay is in samples
    //
    float read(float samples_ago) {
        assert(!data.empty());
        assert(samples_ago < data.size());
        assert(samples_ago >= 0);
        float f = next - samples_ago;
        if (f < 0) f += data.size();
        size_t i = static_cast<size_t>(floor(f));
        size_t j = (1 + i) % data.size();
        float t = f - i;
        float v = data[i] * (1 - t) + t * data[j];
        //printf("v:%f samples_ago:%f left:%zu right:%zu t:%f data[i]:%f data[j]:%f\n", v, f, i, j, t, data[i], data[j]);
        return v;
    }
};
class KarplusStrong : public PlaybackRateObserver {
    DelayLine delayLine;
    History history;
    float _beta = 0;
    float _decay = 1;
    float _delay = 0;

 public:
    void pluck(float hertz, float decay, float beta) {
        _beta = beta;
        _decay = decay;
        _delay = samplerate / hertz;
        delayLine.resize(1 + static_cast<size_t>(samplerate / hertz));
        for (size_t i = 0; i < delayLine.size(); ++i) {
            delayLine.write(2.0f * (std::rand() / static_cast<float>(RAND_MAX)) - 1.0f);
        }
    }

    float operator()() {
        if (delayLine.size() == 0) return 0.0;
        float f = delayLine.read(_delay);
        float v = f * _beta + (1 - _beta) * history(f);
        delayLine.write(v);
        return v;
    }
};
class Timer : public PlaybackRateObserver {
    float value = 0;
    float increment = 0; // normalized frequency

 public:
    void frequency(float hertz) {
        increment = hertz / samplerate; 
    }

    bool operator()() {
        value += increment;

        if (value >= 1.0){
            value -= 1.0f;
            return true;
        }
        return false;
    }
};

class CombFeedback : public PlaybackRateObserver {
    DelayLine delayLine;
    float _delay = 0;
    float _feedback = 0;
    public:

    void configure(float seconds, float feedback) {
        _feedback = feedback;
        _delay = seconds * samplerate;
        delayLine.resize(1 + static_cast<size_t>(_delay));
    }

    float operator()(float input) {
        float output = input + _feedback * delayLine.read(_delay);
        delayLine.write(output);
        return output;
    }
};

class AllPass : public PlaybackRateObserver {
    DelayLine delayLine;
    float _gain = 0;
    float _delay = 0;

    public:

    void configure(float seconds, float gain) {
        _gain = gain;
        _delay = seconds * samplerate;
        delayLine.resize(1 + static_cast<size_t>(_delay));
    }

    float operator()(float input) {
        float read = delayLine.read(_delay);
        float vn = input - _gain * read;
        delayLine.write(vn);
        float output = read + vn * _gain;
        return output;
    }
};

class SchroederReverb {
    CombFeedback comb[4];
    AllPass allpass[3];
    public:

    void configure() {
        comb[0].configure(0.06712f, 0.773f);
        comb[1].configure(0.06404f, 0.802f);
        comb[2].configure(0.08212f, 0.753f);
        comb[3].configure(0.09004f, 0.733f);
        allpass[0].configure(0.01388f, 0.7f);
        allpass[1].configure(0.00452f, 0.7f);
        allpass[2].configure(0.00148f, 0.7f);
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
        float output = input - x1 + 0.995f * y1;
        y1 = output;
        x1 = input;
        return output;
    }
};


class Line : public PlaybackRateObserver {
    float _value = 0;
    float _target = 0;
    float _seconds = 0;
    float _increment = 0;

    public:

    void set(float value) {
        _value = value;
        set();
    }

    void set(float target, float seconds) {
        _target = target;
        _seconds = seconds;
        set();
    }

    void set(float value, float target, float seconds) {
        _value = value;
        _target = target;
        _seconds = seconds;
        set();
    }

    void set() {
        _increment = (_target - _value) / (_seconds * samplerate);
    }

    float operator()() {
        std::equal_to<float> foo;
        if (foo(_value, _target)) {
        //if (_value == _target) {
            return _value;
        }
        
        float v = _value;

        _value += _increment;
        
        // if the value has gone past the target, then stop
        if (_increment <= 0) {
            // slope is negative, so 
            if (_value < _target) {
                // we have gone too far; stop
                _value = _target;
            }
        }
        else {
            // slope is posititve
            if (_value > _target) {
                // we have gone too far
                _value = _target;
            }
        }
        
        return v;
    }

    bool done() {
        std::equal_to<float> foo;
        return foo(_value, _target);
    }
};

class AttackDecay : public PlaybackRateObserver {
    Line up, down;

    int state = 0;

    public:

    void set(float attack_time, float decay_time) {
        state = 2; // ramp up state
        up.set(1, attack_time);
        down.set(1, 0, decay_time);
    }

    float operator()() {
        if (state == 2) {
            float value = up();
            if (up.done()) {
                state = 1; // go to the ramp down state
            }
            return value;
        }
        else if (state == 1) {
            // in the ramp down state
            float value = down();
            if (down.done()) {
                state = 0; // the "off" or "done" state
            }
            return value;
        }
        else {
            // not in the up or down state
            // do nothing
        }

        return 0;
    }
};

class MassSpring : public PlaybackRateObserver {
    float stiffness = 0;
    float damping = 0;
    float position = 0;
    float velocity = 0;
    public:

    void kick() {
        // "kick" the system. displace the mass or impart velocity;
        position = 1; // meters
        velocity = 0; // meters / second

        damping = 0.5f;
        stiffness = 0.1f;
    }
    float operator()() {
        float v = position;

        // Euler's method of numerical integration
        // move the simulation forward...
        float acceleration = -stiffness * position + -damping * velocity;
        velocity += acceleration;
        position += velocity;

        return v;
    }
};
}
