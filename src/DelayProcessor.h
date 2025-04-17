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

            _sampleRate = spec.sampleRate;
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

        void updateRandomly() override
        {
            feedback = 0.3f + rand.nextFloat() * 0.5f;
            float randomFactor = 0.85f + 0.5f * rand.nextFloat();
            delayLine.setDelay(delayTimeSamples * randomFactor);
        }

    private:
        double _sampleRate = 44100;
        const float maxDelaySeconds = 3.0f;
        juce::dsp::DelayLine<float> delayLine { static_cast<int>(maxDelaySeconds)
                                                * static_cast<int>(_sampleRate) }; // Max buffer for 2s at 44.1kHz
        size_t maxDelaySamples = _sampleRate;
        size_t delayTimeSamples = 2400;
        float mix = 0.5f;
        float feedback = 0.5f;
        juce::Random rand;
};