// core_dsp/InputAnalyser.h
#pragma once
class InputAnalyser {
public:
    void prepare(double /*sr*/) {}
    // For testing, we just pretend every signal is 100% tonal (0.0 transient)
    float getTransientScore(float /*input*/) { return 0.0f; } 
};

