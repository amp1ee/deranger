#pragma once

#include <juce_dsp/juce_dsp.h>
#include "ReverbProcessor.h"
#include "DelayProcessor.h"
#include "FlangerProcessor.h"
#include "RoutingNode.h"
#include "signalsmith-stretch.h"

using juce::Reverb;

#define ROOM_SIZE (0.6F)
#define DAMPING   (0.5F)
#define WET_LEVEL (0.9F)
#define DRY_LEVEL (0.7F)

class RackProcessor
{
    public:

        void prepare(const juce::dsp::ProcessSpec &spec)
        {
            root.prepare(spec);
            stretch.presetDefault(static_cast<int>(spec.numChannels),
                                 static_cast<float>(spec.sampleRate));
            stretch.setTransposeSemitones(-5);
        }

        void process(juce::dsp::AudioBlock<float> &block)
        {
            blockCounter++;

            if (stretchEnabled)
                stretchBlock(block);

            // Process the audio block through the routing tree
            root.process(block);

            // Assuming 512-sample buffer @ 44100 Hz → ~11.6 ms per block
            if (toRandomize && (blockCounter % 128) == 0) {
                root.updateRandomly();
            }
        }

        void reset()
        {
            root.reset();
            stretch.reset();
        }

        void addReverb()
        {
            auto reverb = std::make_unique<ReverbProcessor>();

            Reverb::Parameters p;
            p.roomSize = ROOM_SIZE;
            p.damping = DAMPING;
            p.wetLevel = WET_LEVEL;
            p.dryLevel = DRY_LEVEL;
            reverb->setParameters(p);

            auto node = std::make_unique<RoutingNode>();
            node->effect = std::move(reverb);
            root.children.push_back(std::move(node));
        }

        void addDelay()
        {
            auto delay = std::make_unique<DelayProcessor>();

            delay->setDelayTime(4500);
            delay->setFeedback(0.7);

            auto node = std::make_unique<RoutingNode>();
            node->effect = std::move(delay);
            root.children.push_back(std::move(node));
        }

        void addFlanger()
        {
            auto flanger = std::make_unique<FlangerProcessor>();

            flanger->setAmountOfStereo(0.8f);
            flanger->setDelay(10.0f);
            flanger->setFeedback(0.66f);
            flanger->setLFODepth(0.6f);

            auto node = std::make_unique<RoutingNode>();
            node->effect = std::move(flanger);
            root.children.push_back(std::move(node));
        }

        void addEnd() // End of the nodes
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

        [[nodiscard]] bool getRandomize()      const { return this->toRandomize; }
        void setRandomize(bool randomize)            { this->toRandomize = randomize; }
        [[nodiscard]] bool getStretchEnabled() const { return this->stretchEnabled; }
        void setStretchEnabled(bool stretch)            { this->stretchEnabled = stretch; }

        void setBPM(double bpm) { this->currentBPM = bpm; }

        RoutingNode& getRoot() { return this->root; }

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

        bool toRandomize = true;
        bool stretchEnabled = true;
        int blockCounter = 0;
        double currentBPM = 120.0;

        // Vars for stretchBlock():
        int inputSamples, outputSamples, numChannels;
        std::vector<float*> inputPointers;
        std::vector<float*> outputPointers;
        juce::AudioBuffer<float> outputBuffer;
};
