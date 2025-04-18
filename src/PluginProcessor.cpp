#include "./PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EffectRackAudioProcessor::EffectRackAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      )
#endif
{

  rack.addDelay();
  rack.addFlanger();
  rack.addReverb();
}

EffectRackAudioProcessor::~EffectRackAudioProcessor() {}

//==============================================================================
const juce::String EffectRackAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool EffectRackAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool EffectRackAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool EffectRackAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double EffectRackAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int EffectRackAudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int EffectRackAudioProcessor::getCurrentProgram() { return 0; }

void EffectRackAudioProcessor::setCurrentProgram(int index) {}

const juce::String EffectRackAudioProcessor::getProgramName(int  /*index*/) {
  return { "EffectRack by Amplee" };
}

void EffectRackAudioProcessor::changeProgramName(int index,
                                                 const juce::String &newName) {}

//==============================================================================
void EffectRackAudioProcessor::prepareToPlay(double sampleRate,
                                             int samplesPerBlock) {
  // Use this method as the place to do any pre-playback
  // initialisation that you need..

  // Prepare the RackProcessor with the ProcessSpec
  juce::dsp::ProcessSpec spec{};
  spec.sampleRate = sampleRate;
  spec.maximumBlockSize = samplesPerBlock;
  spec.numChannels = getTotalNumOutputChannels();

  // Prepare the RackProcessor (this prepares all modules in the rack)
  rack.prepare(spec);

}

void EffectRackAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
  rack.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EffectRackAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) {
    return false;
}

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet()) {
    return false;
}
#endif

  return true;
#endif
}
#endif

void EffectRackAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                            juce::MidiBuffer & /*midiMessages*/) {
  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  // Clear any unused output channels (same as in your current code)
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
      buffer.clear(i, 0, buffer.getNumSamples());
  }

  // Create AudioBlock from the AudioBuffer for processing
  juce::dsp::AudioBlock<float> block(buffer);

  // Process the block with the RackProcessor
  rack.process(block);

}

//==============================================================================
bool EffectRackAudioProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *EffectRackAudioProcessor::createEditor() {
  return new EffectRackAudioProcessorEditor(*this);
}

//==============================================================================
void EffectRackAudioProcessor::getStateInformation(
    juce::MemoryBlock &destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
}

void EffectRackAudioProcessor::setStateInformation(const void *data,
                                                   int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new EffectRackAudioProcessor();
}
