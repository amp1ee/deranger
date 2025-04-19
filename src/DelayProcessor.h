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
        }

        [[nodiscard]] float getDelayTime() const { return delayTimeSamples; }

        void setDelayTime(float millis, double sampleRate)
        {
            delayTimeSamples = static_cast<float>((millis / 1000.0f) * sampleRate);
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

        float getFeedback() { return feedback; }
        void setFeedback(float fb) { feedback = juce::jlimit(0.0f, 1.0f, fb); }

        void updateRandomly() override
        {
            printf("\n%s: Updating randomly\n", __FILE__);
            setFeedback(0.4f + rand.nextFloat() * 0.4f); // 0.4 - 0.8;
            float randomFactor = 0.85f + 33.0f * rand.nextFloat();
            setDelayTime(maxDelaySeconds * randomFactor, _sampleRate);
        }

        std::string getName() override { return "Delay"; };

    private:
        double _sampleRate = 44100;
        const float maxDelaySeconds = 3.0f;
        juce::dsp::DelayLine<float> delayLine { static_cast<int>(maxDelaySeconds)
                                                * static_cast<int>(_sampleRate) }; // Max buffer for 2s at 44.1kHz
        float maxDelaySamples = _sampleRate;
        float delayTimeSamples = 2400;
        float mix = 0.5f;
        float feedback = 0.5f;
        juce::Random rand;
};