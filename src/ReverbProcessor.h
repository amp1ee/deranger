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

        [[nodiscard]] juce::dsp::Reverb::Parameters getParameters() const {
            return reverb.getParameters();
        }

        void setParameters(const juce::dsp::Reverb::Parameters &params)
        {
            reverb.setParameters(params);
        }

        void updateRandomly() override
        {
            printf("\n%s: Updating randomly\n", __FILE_NAME__);
            float roomSize = 0.4f + rand.nextFloat() * 0.5f;  // 0.4 - 0.9
            float damping = 0.1f + rand.nextFloat() * 0.7f;   // 0.1 - 0.8
            float wet     = 0.2f + rand.nextFloat() * 0.8f;   // 0.2 - 1.0

            juce::dsp::Reverb::Parameters p;
            p.roomSize = roomSize;
            p.damping  = damping;
            p.wetLevel = wet;

            reverb.setParameters(p);
        }

        std::string getName() override { return "Reverb"; };

    private:
        juce::dsp::Reverb reverb;
        juce::Random rand;
};