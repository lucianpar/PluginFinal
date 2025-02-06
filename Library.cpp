#include "Library.h"

void ClipPlayer::addSample(float f) {
    data.push_back(f);
}

// input on (0, 1)
// linear interpolation
float ClipPlayer::operator()(float phase) {
    float index = phase * data.size();
    unsigned a = static_cast<unsigned>(index);
    unsigned b = (a == (data.size() - 1)) ? 0 : a + 1;
    float t = index - a;
    return data[b] * t + (1 - t) * data[a];
}


void Ramp::frequency(float hertz, float samplerate) {
    increment = hertz/samplerate; 
}

float Ramp::operator()() {
    float v = value;

    value += increment;

    if(value >= 1.0){
        value -= 1.0f;
    }

    return v;
}

double sin7(double x) {   
    return x*(x*(x*(x*(x*(x*(194.27296-55.50656*x)-213.704224)+48.57816)+31.77344)+0.89816)-6.311936);                                                                              return x*(x*(x*(x*(x*(x*(194.27296-55.50656*x)-213.704224)+48.57816)+31.77344)+0.89816)-6.311936    );                                 
} 

float History::operator()(float f) {
    float v = history;
    history = f;
    return v;
}

void DelayLine::resize(float seconds, float samplerate) {
    data.resize(1 + static_cast<size_t>(seconds * samplerate));
}

void DelayLine::write(float f) {
    assert(!data.empty());
    //printf("%f\n", f);
    data[next] = f;
    next = (1 + next) % data.size();
}

// delay is in samples
//
float DelayLine::read(float delay) {
    assert(!data.empty());
    assert(delay < data.size());
    float f = next - delay;
    int i = floor(f);
    int j = 1 + i;
    float t = f - i;
    if (i < 0) i += data.size();
    if (j < 0) j += data.size();
    float v = data[i] * (1 - t) + t * data[j];
    //printf("v:%f delay:%f left:%d right:%d t:%f data[i]:%f data[j]:%f\n", v, f, i, j, t, data[i], data[j]);
    return v;
}

size_t DelayLine::size() { return data.size(); }

void KarplusStrong::pluck(float hertz, float samplerate, float decay, float beta) {
    this->beta = beta;
    this->decay = decay;
    this->delay = samplerate / hertz;
    delayLine.resize(1 / hertz, samplerate);
    for (int i = 0; i < delayLine.size(); ++i) {
        delayLine.write(2.0f * (std::rand() / static_cast<float>(RAND_MAX)) - 1.0f);
    }
}

float KarplusStrong::operator()() {
    if (delayLine.size() == 0) return 0.0;
    float f = delayLine.read(delay);
    float v = f * beta + (1 - beta) * history(f);
    delayLine.write(v);
    return v;
}

void Timer::frequency(float hertz, float samplerate) {
    increment = hertz/samplerate; 
}

bool Timer::operator()() {
    value += increment;

    if(value >= 1.0){
        value -= 1.0f;
        return true;
    }
    return false;
}
