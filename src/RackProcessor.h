#pragma once

#include <juce_dsp/juce_dsp.h>
#include "ReverbProcessor.h"
#include "DelayProcessor.h"
#include "FlangerProcessor.h"
#include "RoutingNode.h"
#include "signalsmith-stretch.h"

using juce::Reverb;

class RackProcessor
{
    public:

        void prepare(const juce::dsp::ProcessSpec &spec)
        {
            _sampleRate = static_cast<float>(spec.sampleRate);
            root.prepare(spec);
            stretch.presetDefault(static_cast<int>(spec.numChannels),
                                 static_cast<float>(spec.sampleRate));
            stretch.setTransposeSemitones(stretchSemitones);
            limiter.setThreshold(-4.0f);
            limiter.prepare(spec);
        }

        void process(juce::dsp::AudioBlock<float> &block)
        {
            blockCounter++;

            if (stretchEnabled)
                stretchBlock(block);

            // Process the audio block through the routing tree
            root.process(block);

            if (!juce::JUCEApplicationBase::isStandaloneApp()) {
                secondsPerBeat = 60.0f / currentBPM;
                blockSamples = block.getNumSamples();
                secondsPerBlock = blockSamples / _sampleRate;
                blocksPerBeat = secondsPerBeat / secondsPerBlock;
                blocksPerUpdate = (int)(blocksPerBeat * 4);
            } else
                blocksPerUpdate = 128;
            // Assuming 512-sample buffer @ 44100 Hz → ~11.6 ms per block
            if (toRandomize && (blockCounter % blocksPerUpdate) == 0) {
                root.updateRandomly(currentBPM);
            }

            limiter.process(juce::dsp::ProcessContextReplacing<float>(block));
        }

        void reset()
        {
            root.reset();
            stretch.reset();
            limiter.reset();
        }

        void addReverb(juce::AudioProcessorValueTreeState& params)
        {
            auto reverb = std::make_unique<ReverbProcessor>();

            Reverb::Parameters p;
            p.roomSize = params.getRawParameterValue("roomSize")->load();
            p.damping  = params.getRawParameterValue("damping")->load();
            p.wetLevel = params.getRawParameterValue("wetLevel")->load();;
            p.dryLevel = 1.0f;
            reverb->setParameters(p);

            auto node = std::make_unique<RoutingNode>();
            node->effect = std::move(reverb);
            root.children.push_back(std::move(node));
        }

        void addDelay(juce::AudioProcessorValueTreeState& params)
        {
            auto delay = std::make_unique<DelayProcessor>();

            delay->setDelayTime(params.getRawParameterValue("delayTime")->load() * _sampleRate);
            delay->setFeedback(params.getRawParameterValue("delayFeedback")->load());

            auto node = std::make_unique<RoutingNode>();
            node->effect = std::move(delay);
            root.children.push_back(std::move(node));
        }

        void addFlanger(juce::AudioProcessorValueTreeState& params)
        {
            auto flanger = std::make_unique<FlangerProcessor>();

            flanger->setAmountOfStereo(0.8f);
            flanger->setDelay(params.getRawParameterValue("flangerDelay")->load());
            flanger->setFeedback(params.getRawParameterValue("flangerFeedback")->load());
            flanger->setLFODepth(0.6f);

            auto node = std::make_unique<RoutingNode>();
            node->effect = std::move(flanger);
            root.children.push_back(std::move(node));
        }

        void addEnd() // Marking the end of node tree
        {
            auto node = std::make_unique<RoutingNode>();
            node->effect = nullptr;
            root.children.push_back(std::move(node));
        }

        void printTree(RoutingNode* node, int indent = 0) {
            for (int i = 0; i < indent; ++i) std::cout << "  ";
            std::cout << "Node ID: " << node->getId() << ", children: " << node->children.size() << std::endl;
        
            for (auto& child : node->children)
                printTree(child.get(), indent + 1);
        }

        [[nodiscard]] bool getRandomize()         const { return this->toRandomize; }
        void setRandomize(bool randomize)               { this->toRandomize = randomize; }
        [[nodiscard]] bool getStretchEnabled()    const { return this->stretchEnabled; }
        void setStretchEnabled(bool stretch)            { this->stretchEnabled = stretch; }
        [[nodiscard]] float getStretchSemitones() const { return this->stretchSemitones; }
        void setStretchSemitones(float semitones) {
            this->stretchSemitones = semitones;
            this->stretch.setTransposeSemitones(semitones);
        }

        void setBPM(double bpm) { this->currentBPM = bpm; }

        RoutingNode& getRoot() { return this->root; }

        RackEffect* findProcessor(const juce::String& name)
        {
            for (auto& child : root.children)
            {
                if (child->effect && child->effect->getName() == name)
                    return (child->effect.get());
            }
            return nullptr;
        }

    protected:

        void stretchBlock(juce::dsp::AudioBlock<float> &block) {
            // Determine input and output sample counts
            inputSamples = static_cast<int>(block.getNumSamples()); 
            outputSamples = inputSamples; // Adjust as needed for time-stretching
            numChannels = static_cast<int>(block.getNumChannels());
 
            // Prepare input pointers
            if (inputPointers.size() != numChannels) {
                inputPointers.resize(numChannels);
                outputPointers.resize(numChannels);
            }
            for (int ch = 0; ch < numChannels; ++ch) {
                inputPointers[ch] = block.getChannelPointer(ch);
            }

            // Resize output buffer only if necessary (e.g., if numChannels or outputSamples changes)
            if (outputBuffer.getNumChannels() != numChannels || outputBuffer.getNumSamples() != outputSamples) {
                outputBuffer.setSize(numChannels, outputSamples);
                outputBuffer.clear();
            }        
            // Prepare output pointers
            for (int ch = 0; ch < numChannels; ++ch) {
                outputPointers[ch] = outputBuffer.getWritePointer(ch);
            }

            // Process with Signalsmith Stretch
            stretch.process(inputPointers.data(), inputSamples, outputPointers.data(), outputSamples);

            // Copy processed data back to the original block 
            for (int ch = 0; ch < numChannels; ++ch) {
                std::memcpy(block.getChannelPointer(ch), outputPointers[ch], outputSamples * sizeof(float));
            }
        }

    private:
        RoutingNode root;
        signalsmith::stretch::SignalsmithStretch<float> stretch;
        juce::dsp::Limiter<float> limiter;

        bool toRandomize = true;
        bool stretchEnabled = true;
        float stretchSemitones = -5.0f;
        int blockCounter = 0;
        int blocksPerUpdate = 128;
        int blockSamples = 0;

        float secondsPerBeat, secondsPerBlock;
        float beatsPerUpdate = 1.0f, blocksPerBeat = 1.0f;
        float _sampleRate = 44100.0f;
        double currentBPM = 1.0;

        // Vars for stretchBlock():
        int inputSamples, outputSamples, numChannels;
        std::vector<float*> inputPointers;
        std::vector<float*> outputPointers;
        juce::AudioBuffer<float> outputBuffer;
};
