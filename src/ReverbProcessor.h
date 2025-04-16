#pragma once

#include <JuceHeader.h>

class ReverbProcessor
{
    public:
        ReverbProcessor() = default;

        void prepare(const juce::dsp::ProcessSpec &spec)
        {
            reverb.prepare(spec);
        }

        void process(juce::dsp::AudioBlock<float> &block)
        {
            juce::dsp::ProcessContextReplacing<float> context(block);
            reverb.process(context);
        }

        void reset()
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