#pragma once

#include <JuceHeader.h>
#include "RackEffect.h"

class RoutingNode {
private:
    unsigned int ownId;
    bool isParallel = false;

    juce::AudioBuffer<float> mixBuffer;
    juce::AudioBuffer<float> tmpBuf;
    juce::dsp::AudioBlock<float> tempBlock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RoutingNode)

public:
    RoutingNode();
    ~RoutingNode();

    std::unique_ptr<RackEffect> effect;
    std::vector<std::unique_ptr<RoutingNode>> children;
    std::function<void(RackEffect* effect, const std::string& name)> onEffectParamsChanged{};

    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::dsp::AudioBlock<float>& block);

    unsigned     getId();
    RoutingNode& get(const unsigned id);

    void updateRandomly();
    void setParallel(bool parallel);
    void reset();

    std::string getName();
};
