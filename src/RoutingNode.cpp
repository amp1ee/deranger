#include "RoutingNode.h"

static unsigned int instanceCounter = 0;

RoutingNode::RoutingNode() {
    ownId = ++instanceCounter;
    effect = nullptr;
    printf("Creating node instance #%u\n", instanceCounter);
}

RoutingNode::~RoutingNode() {}

void RoutingNode::prepare(const juce::dsp::ProcessSpec& spec) {
    if (effect) { 
        effect->prepare(spec);

        mixBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
        mixBuffer.clear();
        tmpBuf.setSize(spec.numChannels, spec.maximumBlockSize);
        tmpBuf.clear();
        return; 
    }

    for (auto& child : children)
        child->prepare(spec);

    printf("Node children count: %zu\n", children.size());
}

/**
*   RoutingNode structure:
*       
*                Root Node
*                ____|_____
*               /  |    |  \       
*              /   |    |   \
*             /    |    |    \   
*           FX1   FX2  FX3   FX4
*/
void RoutingNode::process(juce::dsp::AudioBlock<float>& block) {
    if (effect && !isParallel) { 
        effect->process(block);
        return;
    }

    if (!children.empty()) {
        if (isParallel) {
            const auto numSamples = block.getNumSamples();
            auto numChannels = block.getNumChannels();
            // Clear the mixBuffer before mixing
            mixBuffer.setSize(numChannels, numSamples, false, false, true);
            mixBuffer.clear();
            auto mixBlock = juce::dsp::AudioBlock<float>(mixBuffer);

            for (auto& child : children) {
                // Prepare temporary buffer for child output
                tmpBuf.setSize(numChannels, numSamples, false, false, true);
                tmpBuf.clear();

                juce::dsp::AudioBlock<float> tmpBlock(tmpBuf);

                juce::dsp::ProcessContextReplacing<float> context(block);

                // Let the child process into tmpBlock
                if (child->effect)
                    child->effect->process(context);

                // Mix tmpBlock into final mixBuffer
                for (size_t ch = 0; ch < block.getNumChannels(); ++ch) {
                    mixBlock.getChannelPointer(ch); // just ensure the block is ready
                    for (int i = 0; i < numSamples; ++i) {
                        mixBlock.getChannelPointer(ch)[i] += tmpBlock.getChannelPointer(ch)[i];
                    }
                }
            }

            // Overwrite original block with the mixed result
            block = mixBlock;

        } else {
            for (auto& child : children)
                child->process(block);
        }
    }
}

unsigned     RoutingNode::getId() {
    return ownId;
}

RoutingNode& RoutingNode::get(const unsigned id) {
    for (auto& child: children)
        if (child->getId() == id)
            return *child;
    throw std::runtime_error("RoutingNode with ID " + std::to_string(id) + " not found");
}

void RoutingNode::updateRandomly(float bpm) {
    if (effect) { 
        effect->updateRandomly(bpm);
        return;
    }

    for (auto& child : children) {
        child->updateRandomly(bpm);

        // Notify the sliders of the effect parameters change
        if (child->effect && onEffectParamsChanged) {
            juce::MessageManager::callAsync([callback = onEffectParamsChanged,
                                            ptr = child->effect.get(),
                                            name = child->effect->getName()] {
                callback(ptr, name);
            });
        }
    }
}

bool RoutingNode::getParallel() {
    return isParallel;
}

void RoutingNode::setParallel(bool parallel) {
    isParallel = parallel;
}

void RoutingNode::reset() {
    if (effect) { effect->reset(); return; }
    for (auto& child : children)
        child->reset();
}

std::string RoutingNode::getName() { return (effect) ? effect->getName() : "EmptyNode"; }
