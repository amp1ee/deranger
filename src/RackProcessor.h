#pragma once
#include <juce_dsp/juce_dsp.h>  // Add this at the top

#include "ReverbProcessor.h"
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
            for (auto &module : modules)
                module->process(block);
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

    private:
        std::vector<std::unique_ptr<ReverbProcessor>> modules;
};
