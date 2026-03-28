// core_dsp/C_Granular.h
#pragma once
class C_Granular {
public:
    void prepare(double /*sr*/) {}
    float process(float /*transientScore*/) { return 0.0f; } // Silent for now
};

