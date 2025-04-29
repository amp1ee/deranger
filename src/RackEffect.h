#pragma once

#include <JuceHeader.h>

class RackEffect
{
    public:
        virtual ~RackEffect() = default;

        const std::array<float, 4> subdivisions = { 1.0f, 0.5f, 0.75f, 1.5f / 3.0f };

        virtual void prepare(const juce::dsp::ProcessSpec& spec) = 0;
        virtual void process(juce::dsp::AudioBlock<float>& block) = 0;
        virtual void process(juce::dsp::ProcessContextReplacing<float>& context) = 0;
        virtual void reset() {}
        virtual void updateRandomly(float bpm) {}
        virtual std::string getName() { return nullptr; }
};