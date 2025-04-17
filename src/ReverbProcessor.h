#pragma once

#include <JuceHeader.h>
#include "RackEffect.h"

class ReverbProcessor : public RackEffect
{
    public:
        ReverbProcessor() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) override
        {
            reverb.prepare(spec);
        }

        void process(juce::dsp::AudioBlock<float> &block) override
        {
            juce::dsp::ProcessContextReplacing<float> context(block);
            reverb.process(context);
        }

        void reset() override
        {
            reverb.reset();
        }

        void setParameters(const juce::dsp::Reverb::Parameters &params)
        {
            reverb.setParameters(params);
        }

    private:
        juce::dsp::Reverb reverb;
};