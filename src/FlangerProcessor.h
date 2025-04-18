#pragma once

#include <JuceHeader.h>
#include "RackEffect.h"

class FlangerProcessor : public RackEffect
{
public:
    FlangerProcessor() = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        _sampleRate = spec.sampleRate;
        numChannels = spec.numChannels;

        const float maxDelayInSeconds = (maxDepth * maximumDelayModulation + maxCentreDelayMs) / 1000.0f;
        const int delayBufferSize = static_cast<int>(std::ceil(_sampleRate * maxDelayInSeconds));

        flangerDelay = juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>(delayBufferSize);
        flangerDelay.prepare(spec);

        mixer.prepare(spec);
        feedback.resize(numChannels);
        lfo.prepare(spec);

        lfo.setFrequency(lfoFreq);
        lfo.initialise([](float val) { return std::sin(val); });
        modTimesBuffer.setSize(1, static_cast<int>(spec.maximumBlockSize));

        smoothedDelay.reset(_sampleRate, 2.5f);
        smoothedLFODepth.reset(_sampleRate, 2.5f);
        smoothedFeedback.reset(_sampleRate, 2.5f);
        reset();
    }

    [[nodiscard]] float getAmountOfStereo() const { return stereoWidth; }
    [[nodiscard]] float getDelay()                { return smoothedDelay.getNextValue(); }
    [[nodiscard]] float getLFODepth()             { return smoothedLFODepth.getNextValue(); }
    [[nodiscard]] float getFeedback()             { return smoothedFeedback.getNextValue(); }

    void setAmountOfStereo(float newWidth) { stereoWidth = newWidth; }
    void setDelay(float newCentreDelayMs)  { smoothedDelay.setTargetValue(newCentreDelayMs); }
    void setLFODepth(float newLfoDepth) { smoothedLFODepth.setTargetValue(newLfoDepth); }
    void setFeedback(float newFeedback) { smoothedFeedback.setTargetValue(newFeedback); }

    void process(juce::dsp::AudioBlock<float> &block) override
    {
        juce::dsp::ProcessContextReplacing<float> context(block);

        const auto &inputBlock = context.getInputBlock();
        auto &outputBlock = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples = outputBlock.getNumSamples();
    
        jassert(inputBlock.getNumChannels() == numChannels);
        jassert(inputBlock.getNumChannels() == feedback.size());
        jassert(inputBlock.getNumSamples() == numSamples);
    
        mixer.pushDrySamples(inputBlock);
    
        for (size_t channel = 0; channel < numChannels; ++channel) {
            auto *inputSamples = inputBlock.getChannelPointer(channel);
            auto *outputSamples = outputBlock.getChannelPointer(channel);
            float phaseOffset = (channel == 1) ? juce::MathConstants<float>::halfPi * getAmountOfStereo() : 0.0f;
    
            for (size_t i = 0; i < numSamples; ++i) {
                float input = inputSamples[i];
    
                float lfoValue = lfo.processSample(phaseOffset);
                float delayCalc = (getDelay() + lfoValue * getLFODepth()) * (static_cast<float>(_sampleRate) / 1000.0f);
                flangerDelay.setDelay(delayCalc);
    
                float feedbackSign = 1.0f;
                float inputWithFeedback = input + (feedbackSign * feedback[channel]);
                flangerDelay.pushSample(static_cast<int>(channel), inputWithFeedback);
                float wetSignal = flangerDelay.popSample(static_cast<int>(channel));
    
                outputSamples[i] = wetSignal;
                feedback[channel] = wetSignal * getFeedback();
            }
        }
    
        mixer.mixWetSamples(outputBlock);
    }

    void reset() override
    {
        std::fill(feedback.begin(), feedback.end(), static_cast<float>(0));
        flangerDelay.reset();
        lfo.reset();
        mixer.reset();
    }

    void updateRandomly() override
    {
        printf("\n%s: Updating randomly\n", __FILE__);

        this->setDelay(juce::Random::getSystemRandom().nextFloat() * 0.25f);
        this->setLFODepth(juce::Random::getSystemRandom().nextFloat() * 0.5f);
        this->setFeedback(juce::Random::getSystemRandom().nextFloat() * 0.75f);
    }

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> flangerDelay;
    juce::dsp::Oscillator<float> lfo;

    double _sampleRate = 44100.0f;
    int numChannels = 0;
    float maxDelayTime = 0.02f;
    float lfoDepth = 0.5f;
    float lfoFreq = 0.33f;
    float stereoWidth = 0.6f;

    float   maxDepth               = 1.0f,
            maxCentreDelayMs       = 100.0f,
            maximumDelayModulation = 20.0f;

    std::vector<float> feedback{0.5f};
    juce::AudioBuffer<float> modTimesBuffer;
    juce::dsp::DryWetMixer<float> mixer;
    juce::LinearSmoothedValue<float> smoothedDelay = { maxCentreDelayMs };
    juce::LinearSmoothedValue<float> smoothedLFODepth = { lfoDepth };
    juce::LinearSmoothedValue<float> smoothedFeedback = { feedback[0] };
};