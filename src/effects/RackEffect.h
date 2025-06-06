#pragma once

#include <JuceHeader.h>

class RackEffect
{
    public:
        virtual ~RackEffect() = default;

        virtual void prepare(const juce::dsp::ProcessSpec& spec) = 0;
        virtual void process(juce::dsp::AudioBlock<float>& block) = 0;
        virtual void process(juce::dsp::ProcessContextReplacing<float>& context) = 0;
        virtual void reset() {}
        virtual void updateRandomly(float bpm) {}
        virtual std::string getName() { return nullptr; }
        [[nodiscard]] virtual bool getParallel() const { return false; }
        [[nodiscard]] virtual std::map<std::string, float> getParameterMap() { return {}; }
};