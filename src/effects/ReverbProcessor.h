#pragma once

#include <JuceHeader.h>
#include "RackEffect.h"

class ReverbProcessor : public RackEffect
{
    public:
        ReverbProcessor() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) override
        {
            reverb.prepare(spec);
        }

        void process(juce::dsp::AudioBlock<float> &block) override
        {
            juce::dsp::ProcessContextReplacing<float> context(block);
            reverb.process(context);
        }

        void process(juce::dsp::ProcessContextReplacing<float>& context) override
        {
            reverb.process(context);
        }

        void reset() override
        {
            reverb.reset();
        }

        [[nodiscard]] juce::dsp::Reverb::Parameters getParameters() const {
            return reverb.getParameters();
        }

        void setParameters(const juce::dsp::Reverb::Parameters &params)
        {
            reverb.setParameters(params);
        }

        void updateRandomly(float /*bpm*/) override
        {
            p = reverb.getParameters();

            if (roomSizeRandomize)
                p.roomSize = 0.4f + rand.nextFloat() * 0.5f; // 0.4 - 0.9
            if (dampingRandomize)
                p.damping = 0.1f + rand.nextFloat() * 0.7f; // 0.1 - 0.8
            if (wetLevelRandomize)
                p.wetLevel = 0.2f + rand.nextFloat() * 0.8f; // 0.2 - 1.0

            reverb.setParameters(p);
        }

        std::string getName() override { return "Reverb"; };

        void setRoomSizeRandomize(bool shouldRandomize) { roomSizeRandomize = shouldRandomize; }
        void setDampingRandomize(bool shouldRandomize) { dampingRandomize = shouldRandomize; }
        void setWetLevelRandomize(bool shouldRandomize) { wetLevelRandomize = shouldRandomize; }

        [[nodiscard]] bool getRoomSizeRandomize() const { return roomSizeRandomize; }
        [[nodiscard]] bool getDampingRandomize() const { return dampingRandomize; }
        [[nodiscard]] bool getWetLevelRandomize() const { return wetLevelRandomize; }

        [[nodiscard]] std::map<std::string, float> getParameterMap() override
        {
            return {
                {"roomSize", p.roomSize},
                {"damping", p.damping},
                {"wetLevel", p.wetLevel}
            };
        }

    private:
        juce::dsp::Reverb reverb;
        juce::Random rand;
        juce::dsp::Reverb::Parameters p;

        bool roomSizeRandomize = true;
        bool dampingRandomize = true;
        bool wetLevelRandomize = true;
};