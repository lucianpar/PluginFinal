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
