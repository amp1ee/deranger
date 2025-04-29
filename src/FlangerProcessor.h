
#pragma once

#include <JuceHeader.h>
#include "RackEffect.h"

class FlangerProcessor : public RackEffect
{
public:
    FlangerProcessor() = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        _sampleRate = static_cast<float>(spec.sampleRate);
        numChannels = static_cast<int>(spec.numChannels);

        const float maxDelayInSeconds = (maxDepth * maximumDelayModulationMs + maxCentreDelayMs) / 1000.0f;
        const int delayBufferSize = static_cast<int>(std::ceil(_sampleRate * maxDelayInSeconds));

        flangerDelay = juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>(delayBufferSize);
        flangerDelay.prepare(spec);

        mixer.prepare(spec);
        feedback.resize(numChannels);
        lfo.prepare(spec);

        lfo.setFrequency(lfoFreq);
        lfo.initialise([](float val) { return std::sin(val); });

        smoothedDelay.reset(_sampleRate, 0.01f);
        smoothedLFODepth.reset(_sampleRate, 0.02f);
        smoothedFeedback.reset(_sampleRate, 0.04f);
        reset();
    }

    [[nodiscard]] float getAmountOfStereo() const { return stereoWidth; }
    [[nodiscard]] float getDelay()                { return smoothedDelay.getNextValue(); }
    [[nodiscard]] float getLFODepth()             { return smoothedLFODepth.getNextValue(); }
    [[nodiscard]] float getFeedback()             { return smoothedFeedback.getNextValue(); }

    void setAmountOfStereo(float newWidth) { stereoWidth = newWidth; }
    void setDelay(float newCentreDelayMs)  { smoothedDelay.setTargetValue(newCentreDelayMs); }
    void setLFODepth(float newLfoDepth)    { smoothedLFODepth.setTargetValue(newLfoDepth); }
    void setFeedback(float newFeedback)    { smoothedFeedback.setTargetValue(newFeedback); }

    void process(juce::dsp::AudioBlock<float> &block) override
    {
        juce::dsp::ProcessContextReplacing<float> context(block);
        inputBlock =  &context.getInputBlock();
        outputBlock = &context.getOutputBlock();
        numSamples = static_cast<int>(outputBlock->getNumSamples());

        mixer.pushDrySamples(*inputBlock);

        for (int channel = 0; channel < numChannels; ++channel) {

            phaseOffset = (channel == 1) ? juce::MathConstants<float>::halfPi * getAmountOfStereo() : 0.0f;

            for (int i = 0; i < numSamples; ++i) {
                input = inputBlock->getSample(channel, i);

                lfoValue = lfo.processSample(phaseOffset);
                delayCalcMs = juce::jlimit(1.0f, 20.0f, getDelay() + (lfoValue * getLFODepth()));
                delayCalcSamples = delayCalcMs * (_sampleRate / 1000.0f);
                flangerDelay.setDelay(delayCalcSamples);

                inputWithFeedback = input + feedback[channel];
                flangerDelay.pushSample(channel, inputWithFeedback);
                wetSignal = flangerDelay.popSample(channel);

                outputBlock->setSample(channel, i, wetSignal);
                feedback[channel] = wetSignal * getFeedback();
            }
        }

        mixer.mixWetSamples(*outputBlock);
    }

    void process(juce::dsp::ProcessContextReplacing<float>& context) override
    {
        process(context.getOutputBlock());
    }

    void reset() override
    {
        std::fill(feedback.begin(), feedback.end(), static_cast<float>(0));
        flangerDelay.reset();
        lfo.reset();
        mixer.reset();
    }

    void updateRandomly(float bpm) override
    {
        if (delayRandomize)
        {                
            if (bpm > 1.0f)
            {
                static const Array<float> fineSubdivisions = {
                    1.0f / 128.0f,
                    1.0f / 64.0f,
                    1.0f / 48.0f,
                    1.0f / 32.0f,
                    1.0f / 24.0f,
                    1.0f / 16.0f,
                    1.0f / 12.0f,
                    1.0f / 8.0f
                };
        
                Array<float> validSubdivisions;
                for (float subdivision : fineSubdivisions)
                {
                    float delayMs = (60.0f / bpm) * subdivision * 1000.0f;
                    if (delayMs <= maxCentreDelayMs)
                        validSubdivisions.add(subdivision);
                }
        
                if (!validSubdivisions.isEmpty())
                {
                    float chosenSubdivision = validSubdivisions[rand.nextInt(validSubdivisions.size())];
                    float delayMs = (60.0f / bpm) * chosenSubdivision * 1000.0f;
                    setDelay(delayMs);
                }
                else
                {
                    setDelay(rand.nextFloat() * maxCentreDelayMs);
                }
            }
            else
            {
                setDelay(rand.nextFloat() * maxCentreDelayMs);
            }
        }
        if (depthRandomize)
            setLFODepth(juce::Random::getSystemRandom().nextFloat() * maxDepth);
        if (feedbackRandomize)
            setFeedback(juce::Random::getSystemRandom().nextFloat());
    }

    std::string getName() override { return "Flanger"; };

    void setDelayRandomize(bool shouldRandomize) { delayRandomize = shouldRandomize; }
    void setDepthRandomize(bool shouldRandomize) { depthRandomize = shouldRandomize; }
    void setFeedbackRandomize(bool shouldRandomize) { feedbackRandomize = shouldRandomize; }

    [[nodiscard]] bool getDelayRandomize() const { return delayRandomize; }
    [[nodiscard]] bool getDepthRandomize() const { return depthRandomize; }
    [[nodiscard]] bool getFeedbackRandomize() const { return feedbackRandomize; }

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> flangerDelay;
    juce::dsp::Oscillator<float> lfo;

    float _sampleRate = 44100.0f;
    int numChannels = 0, numSamples;

    float lfoDepth = 5.0f; // max mod depth ~5ms
    float lfoFreq = 0.33f;
    float stereoWidth = 0.6f;

    float maxDepth = 1.0f;
    float maxCentreDelayMs = 15.0f;            // Reduced from 300ms to 15ms
    float maximumDelayModulationMs = 5.0f;     // Max mod depth ~5ms

    // Preallocations ahead of the process loop:
    float wetSignal, inputWithFeedback, lfoValue, delayCalcMs,
                    delayCalcSamples, input, phaseOffset;
    const juce::dsp::AudioBlock<const float> *inputBlock;
    juce::dsp::AudioBlock<float> *outputBlock;

    std::vector<float> feedback{0.5f};
    juce::Random rand;

    juce::dsp::DryWetMixer<float> mixer;
    juce::LinearSmoothedValue<float> smoothedDelay = { maxCentreDelayMs };
    juce::LinearSmoothedValue<float> smoothedLFODepth = { lfoDepth };
    juce::LinearSmoothedValue<float> smoothedFeedback = { feedback[0] };

    bool delayRandomize = true;
    bool depthRandomize = true;
    bool feedbackRandomize = true;
};
