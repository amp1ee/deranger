#pragma once

#include <JuceHeader.h>
#include "RackEffect.h"

class RoutingNode {
public:

    RoutingNode() = default;

    std::unique_ptr<RackEffect> effect;
    std::vector<std::unique_ptr<RoutingNode>> children;
    std::function<void(RackEffect* effect, const std::string& name)> onEffectParamsChanged{};

    bool isParallel = false;

    void prepare(const juce::dsp::ProcessSpec& spec) {
        if (effect) effect->prepare(spec);
        for (auto& child : children)
            child->prepare(spec);
    }

    void process(juce::dsp::AudioBlock<float>& block) {
        if (effect) effect->process(block);

        if (!children.empty()) {
            if (isParallel) {
                juce::AudioBuffer<float> mixBuffer;
                mixBuffer.setSize(block.getNumChannels(), block.getNumSamples());
                mixBuffer.clear();

                for (auto& child : children) {
                    juce::AudioBuffer<float> tmpBuf;
                    tmpBuf.setSize((int)block.getNumChannels(), (int)block.getNumSamples());
                    for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
                        tmpBuf.copyFrom((int)ch, 0, block.getChannelPointer(ch), (int)block.getNumSamples());
                    juce::dsp::AudioBlock<float> tempBlock(tmpBuf);
                    child->process(tempBlock);
                    for (int ch = 0; ch < mixBuffer.getNumChannels(); ++ch)
                        mixBuffer.addFrom(ch, 0, tmpBuf, ch, 0, tmpBuf.getNumSamples());
                }

                juce::dsp::AudioBlock<float> mixedBlock(mixBuffer);
                block = mixedBlock;
            } else {
                for (auto& child : children)
                    child->process(block);
            }
        }
    }

    void updateRandomly() {
        if (effect) { 
            effect->updateRandomly();
            return;
        }

        for (auto& child : children) { // randomize rest of the nodes recursively:
            child->updateRandomly();

            if (child->effect && onEffectParamsChanged) {
                juce::MessageManager::callAsync([callback = onEffectParamsChanged,
                                                ptr = child->effect.get(),
                                                name = child->effect->getName()] {
                    callback(ptr, name);
                });
            }
        }
    }

    void reset() {
        if (effect) effect->reset();
        for (auto& child : children)
            child->reset();
    }

    std::string getName() { return (effect) ? effect->getName() : "EmptyNode"; }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RoutingNode)
};
