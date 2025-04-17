#pragma once

#include <JuceHeader.h>
#include "RackEffect.h"

class DelayProcessor: public RackEffect
{
    public:
        DelayProcessor() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) override
        {
            delayLine.reset();
            delayLine.prepare(spec);
            maxDelaySamples = static_cast<size_t>(spec.sampleRate * maxDelaySeconds);
            delayLine.setMaximumDelayInSamples(maxDelaySamples);
            delayLine.setDelay(delayTimeSamples);
        }

        void setDelayTime(float millis, double sampleRate)
        {
            delayTimeSamples = static_cast<size_t>((millis / 1000.0f) * sampleRate);
            delayLine.setDelay(delayTimeSamples);
        }

        void process(juce::dsp::AudioBlock<float> &block) override
        {
            for (size_t ch = 0; ch < block.getNumChannels(); ch++)
            {
                auto *channelData = block.getChannelPointer(ch);

                for (size_t i = 0; i < block.getNumSamples(); i++)
                {
                    float in = channelData[i];
                    float delayed = delayLine.popSample(ch);
                    delayLine.pushSample(ch, in + feedback * delayed);
                    channelData[i] = mix * delayed + (1.0f - mix) * in;
                }
            }
        }

        void reset() override { delayLine.reset(); }
    
        void setMix(float newMix) { mix = juce::jlimit(0.0f, 1.0f, newMix); }
        void setFeedback(float fb) { feedback = juce::jlimit(0.0f, 1.0f, fb); }

    private:
        juce::dsp::DelayLine<float> delayLine { 88200 }; // Max buffer for 2s at 44.1kHz
        size_t maxDelaySamples = 44100;
        const float maxDelaySeconds = 2.0f;
        size_t delayTimeSamples = 2400;
        float mix = 0.5f;
        float feedback = 0.5f;
};