#include "RoutingNode.h"

static unsigned int instanceCounter = 0;

RoutingNode::RoutingNode() {
    ownId = ++instanceCounter;
    printf("Creating node instance #%u\n", instanceCounter);
}

RoutingNode::~RoutingNode() {}

void RoutingNode::prepare(const juce::dsp::ProcessSpec& spec) {
    if (effect) { 
        effect->prepare(spec);

        mixBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
        tmpBuf.setSize(spec.numChannels, spec.maximumBlockSize);
        return; 
    }

    for (auto& child : children)
        child->prepare(spec);

    printf("Node children count: %zu\n", children.size());
}

void RoutingNode::process(juce::dsp::AudioBlock<float>& block) {
    if (effect && !isParallel) { effect->process(block); return; }

    if (!children.empty()) {
        if (isParallel) {
            
            mixBuffer.setSize(block.getNumChannels(), block.getNumSamples());
            mixBuffer.clear();

            for (auto& child : children) {
                tmpBuf.setSize((int)block.getNumChannels(), (int)block.getNumSamples());
                for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
                    tmpBuf.copyFrom((int)ch, 0, block.getChannelPointer(ch), (int)block.getNumSamples());
                tempBlock = juce::dsp::AudioBlock<float>(tmpBuf);

                printf("Processing child %d\n", child->getId());
                child->process(tempBlock);
                printf("Child %d processed\n", child->getId());
                for (int ch = 0; ch < tmpBuf.getNumChannels(); ++ch)
                {
                    float sum = 0.0f;
                    for (int s = 0; s < tmpBuf.getNumSamples(); ++s)
                        sum += std::abs(tmpBuf.getSample(ch, s));
                
                    printf(" -> Channel %d energy: %.4f\n", ch, sum);
                }

                for (int ch = 0; ch < mixBuffer.getNumChannels(); ++ch)
                    mixBuffer.addFrom(ch, 0, tmpBuf, ch, 0, tmpBuf.getNumSamples());
            }
            block = juce::dsp::AudioBlock<float>(mixBuffer);
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

void RoutingNode::updateRandomly() {
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

void RoutingNode::setParallel(bool parallel) {
    isParallel = parallel;
}

void RoutingNode::reset() {
    if (effect) { effect->reset(); return; }
    for (auto& child : children)
        child->reset();
}

std::string RoutingNode::getName() { return (effect) ? effect->getName() : "EmptyNode"; }
