#pragma once

#include <juce_dsp/juce_dsp.h>
#include "ReverbProcessor.h"
#include "DelayProcessor.h"
#include "FlangerProcessor.h"
#include "RoutingNode.h"

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
        }

        void process(juce::dsp::AudioBlock<float> &block)
        {
            blockCounter++;
            root.process(block);

            // Assuming 512-sample buffer @ 44100 Hz → ~11.6 ms per block
            if ((blockCounter % 128) == 0) {
                root.updateRandomly();
            }
        }

        void reset()
        {
            root.reset();
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
            flanger->setDelay(300.0f);
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

        RoutingNode& getRoot() { return this->root; }

    private:
        RoutingNode root;
        int blockCounter = 0;
};
