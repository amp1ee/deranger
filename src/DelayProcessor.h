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
            maxDelaySamples = static_cast<float>(spec.sampleRate * maxDelaySeconds);
            delayLine.setMaximumDelayInSamples(maxDelaySamples);
            delayLine.setDelay(delayTimeSamples);

            smoothedDelay.reset(_sampleRate, 0.15f);
            smoothedFeedback.reset(_sampleRate, 0.15f);
        }

        [[nodiscard]] float getDelayTime() { return smoothedDelay.getNextValue(); }

        void setDelayTime(float millis)
        {
            delayTimeSamples = millis;
            smoothedDelay.setTargetValue(delayTimeSamples);
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
                    delayLine.setDelay(smoothedDelay.getNextValue());
                    delayLine.pushSample(ch, in + getFeedback() * delayed);
                    channelData[i] = mix * delayed + (1.0f - mix) * in;
                }
            }
        }

        void reset() override { delayLine.reset(); }
    
        void setMix(float newMix) { mix = juce::jlimit(0.0f, 1.0f, newMix); }

        float getFeedback() { return smoothedFeedback.getNextValue(); }
        void setFeedback(float fb) { smoothedFeedback.setTargetValue(fb); }

        void updateRandomly() override
        {
            printf("\n%s: Updating randomly\n", __FILE_NAME__);
            setFeedback(0.3f + rand.nextFloat() * 0.5f); // 0.3 - 0.8;
            setDelayTime(rand.nextFloat() * maxDelaySamples);
        }

        std::string getName() override { return "Delay"; };

    private:
        double _sampleRate = 44100.0f;
        const float maxDelaySeconds = 3.0f;

        float maxDelaySamples = maxDelaySeconds * _sampleRate;
        juce::dsp::DelayLine<float> delayLine { static_cast<int>(maxDelaySamples) };
        
        float delayTimeSamples = 2400.0f;
        float mix = 0.5f;
        float feedback = 0.5f;
        juce::Random rand;
        juce::LinearSmoothedValue<float> smoothedDelay = { maxDelaySamples };
        juce::LinearSmoothedValue<float> smoothedFeedback = { feedback };
};