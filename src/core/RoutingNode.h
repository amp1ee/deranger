#pragma once

#include <JuceHeader.h>
#include "../effects/RackEffect.h"

/**
*   RoutingNode structure:
*       
*                Root Node
*                ____|_____
*               /  |    |  \       
*              /   |    |   \
*             /    |    |    \   
*           FX1   FX2  FX3   FX4
*
*/

static bool isParallel;

class RoutingNode {
private:
    unsigned int ownId;
    
    juce::AudioBuffer<float>        mixBuffer;
    juce::AudioBuffer<float>        tmpBuf;
    juce::dsp::AudioBlock<float>    tmpBlock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RoutingNode)

public:
    RoutingNode();
    ~RoutingNode();
    
    std::vector<std::unique_ptr<RoutingNode>> children;
    std::unique_ptr<RackEffect> effect;
    std::function<void(RackEffect* effect, const std::string& name)> onEffectParamsChanged{};

    void         prepare(const juce::dsp::ProcessSpec& spec);
    void         process(juce::dsp::AudioBlock<float>& block);
    void         reset();

    RoutingNode& get(const unsigned id);
    unsigned     getId();
    std::string  getName();
    void         updateRandomly(float bpm);
    bool         getParallel();
    void         setParallel(bool parallel);
};
