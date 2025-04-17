#pragma once
#include <juce_dsp/juce_dsp.h>  // Add this at the top

#include "ReverbProcessor.h"
#include "DelayProcessor.h"
#include "RackEffect.h"

using juce::Reverb;

#define ROOM_SIZE (0.6F)
#define DAMPING   (0.5F)
#define WET_LEVEL (0.7F)
#define DRY_LEVEL (1.0F)

class RackProcessor
{
    public:
        void prepare(const juce::dsp::ProcessSpec &spec)
        {
            for (auto &module : modules)
                module->prepare(spec);
        }

        void process(juce::dsp::AudioBlock<float> &block)
        {
            blockCounter++;
            for (auto &module : modules) {
                module->process(block);

                // Assuming 512-sample buffer @ 44100 Hz → ~11.6 ms per block
                if ((blockCounter % 100) == 0) { // ≈ every 1.2 sec
                    module->updateRandomly();
                }                
            }
        }

        void reset()
        {
            for (auto &module : modules)
                module->reset();
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

            modules.push_back(std::move(reverb));
        }

        void addDelay()
        {
            auto delay = std::make_unique<DelayProcessor>();

            delay->setDelayTime(1500, 44100);
            delay->setFeedback(0.7);
            modules.push_back(std::move(delay));

        }

    private:
        std::vector<std::unique_ptr<RackEffect>> modules;
        int blockCounter = 0;
};
