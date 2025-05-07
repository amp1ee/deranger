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
            numChannels = static_cast<int>(spec.numChannels);
            maxDelaySamples = static_cast<float>(spec.sampleRate * maxDelaySeconds);
            delayLine.setMaximumDelayInSamples(static_cast<int>(maxDelaySamples));
            delayLine.setDelay(delayTimeSamples);

            smoothedDelay.reset(_sampleRate, 0.01f);
            smoothedFeedback.reset(_sampleRate, 0.02f);
        }

        [[nodiscard]] float getDelayTime() { return smoothedDelay.getNextValue(); }
        [[nodiscard]] float getTargetDelayTime() { return smoothedDelay.getTargetValue(); }

        void setDelayTime(float samples)
        {
            delayTimeSamples = samples;
            smoothedDelay.setTargetValue(delayTimeSamples);
            delayLine.setDelay(smoothedDelay.getNextValue());
        }

        void process(juce::dsp::AudioBlock<float>& block) override
        {
            numSamples = static_cast<int>(block.getNumSamples());
        
            for (int ch = 0; ch < numChannels; ++ch)
            {
                for (int i = 0; i < numSamples; ++i)
                {
                    in = block.getSample(ch, i);
                    delayed = delayLine.popSample(ch);
                    delayLine.setDelay(smoothedDelay.getNextValue());
                    delayLine.pushSample(ch, in + (getFeedback() * delayed));
                    block.setSample(ch, i, (mix * delayed) + (1.0f - mix) * in);
                }
            }
        }

        void process(juce::dsp::ProcessContextReplacing<float>& context) override
        {
            process(context.getOutputBlock());
        }

        void reset() override { delayLine.reset(); }
    
        void setMix(float newMix) { mix = juce::jlimit(0.0f, 1.0f, newMix); }

        float getFeedback() { return smoothedFeedback.getTargetValue(); }
        void setFeedback(float fb) { smoothedFeedback.setTargetValue(fb); }

        void updateRandomly(float bpm) override
        {
            if (feedbackRandomize)
                setFeedback(0.3f + rand.nextFloat() * 0.5f); // 0.3 - 0.8
            if (delayTimeRandomize)
            {
                if (bpm > 1.0f)
                {
                    
                    const std::array<float, 9> subdivisions = {
                        1.0f, 0.5f, 0.75f, 1.0/3.0f,
                        2.0f, 0.25f, 2.0/3.0f, 1.5f, 3.0f 
                    };

                    // Choose a musical subdivision at random
                    float noteLength = subdivisions[rand.nextInt(subdivisions.size())];
                    float delaySec = (60.0f / bpm) * noteLength;
        
                    setDelayTime(delaySec * static_cast<float>(_sampleRate));
                }
                else
                {
                    setDelayTime(rand.nextFloat() * maxDelaySamples);
                }
            }
        }

        std::string getName() override { return "Delay"; };

        [[nodiscard]] bool getFeedbackRandomize()  const { return this->feedbackRandomize; }
        [[nodiscard]] bool getDelayTimeRandomize() const { return this->delayTimeRandomize; }

        void setFeedbackRandomize(bool shouldRandomize)  { feedbackRandomize = shouldRandomize; }
        void setDelayTimeRandomize(bool shouldRandomize) { delayTimeRandomize = shouldRandomize; }

        [[nodiscard]] std::map<std::string, float> getParameterMap() override
        {
            return {
                { "delayTime",     getTargetDelayTime() / _sampleRate },
                { "delayFeedback", getFeedback() }
            };
        }

    private:
        double _sampleRate = 44100.0f;
        const float maxDelaySeconds = 3.0f;

        float maxDelaySamples = maxDelaySeconds * _sampleRate;
        juce::dsp::DelayLine<float> delayLine { static_cast<int>(maxDelaySamples) };
        
        float delayTimeSamples = 2400.0f;
        float mix = 0.5f;
        float feedback = 0.5f;
        bool feedbackRandomize = true;
        bool delayTimeRandomize = true;

        // Preallocating before the process loop
        int numChannels, numSamples; float in, delayed;
        
        juce::Random rand;
        juce::LinearSmoothedValue<float> smoothedDelay = { maxDelaySamples };
        juce::LinearSmoothedValue<float> smoothedFeedback = { feedback };
};