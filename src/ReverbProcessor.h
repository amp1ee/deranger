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

        void process(juce::dsp::ProcessContextReplacing<float>& context) override
        {
            reverb.process(context);
        }

        void reset() override
        {
            reverb.reset();
        }

        [[nodiscard]] juce::dsp::Reverb::Parameters getParameters() const {
            return reverb.getParameters();
        }

        void setParameters(const juce::dsp::Reverb::Parameters &params)
        {
            reverb.setParameters(params);
        }

        void updateRandomly() override
        {
            p = reverb.getParameters();
            p.roomSize = 0.4f + rand.nextFloat() * 0.5f;  // 0.4 - 0.9
            p.damping =  0.1f + rand.nextFloat() * 0.7f;   // 0.1 - 0.8
            p.wetLevel = 0.2f + rand.nextFloat() * 0.8f;   // 0.2 - 1.0

            reverb.setParameters(p);
        }

        std::string getName() override { return "Reverb"; };

    private:
        juce::dsp::Reverb reverb;
        juce::Random rand;
        juce::dsp::Reverb::Parameters p;
};