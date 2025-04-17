#pragma once

#include <JuceHeader.h>

class RackEffect
{
    public:
        virtual ~RackEffect() = default;

        virtual void prepare(const juce::dsp::ProcessSpec& spec) = 0;
        virtual void process(juce::dsp::AudioBlock<float>& block) = 0;
        virtual void reset() {}
        virtual void updateRandomly() {}
};