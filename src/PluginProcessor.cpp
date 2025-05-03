#include "./PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DerangerAudioProcessor::DerangerAudioProcessor()
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
  rack.addEnd();

  rack.printTree(&rack.getRoot(), 0);

}

DerangerAudioProcessor::~DerangerAudioProcessor() {}

//======= States and Parameters ================================================

void DerangerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::XmlElement xml("DerangerState");

    xml.setAttribute("version", 1);

    if (auto* rackXml = rack.saveToXml().get())
        xml.addChildElement(rackXml);

    copyXmlToBinary(xml, destData);
}

void DerangerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml && xml->hasTagName("DerangerState"))
    {
        int version = xml->getIntAttribute("version", 1);

        if (auto* rackXml = xml->getChildByName("RackState"))
            rack.loadFromXml(*rackXml);
    }
}

//==============================================================================
const juce::String DerangerAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool DerangerAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool DerangerAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool DerangerAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double DerangerAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int DerangerAudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int DerangerAudioProcessor::getCurrentProgram() { return 0; }

void DerangerAudioProcessor::setCurrentProgram(int index) {}

const juce::String DerangerAudioProcessor::getProgramName(int  /*index*/) {
  return { "Deranger by Amplee" };
}

void DerangerAudioProcessor::changeProgramName(int index,
                                                 const juce::String &newName) {}

//==============================================================================
void DerangerAudioProcessor::prepareToPlay(double sampleRate,
                                             int samplesPerBlock) {
  
  _sampleRate = sampleRate;

  // Prepare the RackProcessor with the ProcessSpec
  juce::dsp::ProcessSpec spec{};
  spec.sampleRate = sampleRate;
  spec.maximumBlockSize = samplesPerBlock;
  spec.numChannels = getTotalNumOutputChannels();

  // Prepare the RackProcessor (this prepares all modules in the rack)
  rack.prepare(spec);

}

void DerangerAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
  rack.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DerangerAudioProcessor::isBusesLayoutSupported(
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

void DerangerAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                            juce::MidiBuffer & /*midiMessages*/) {
  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  // Clear any unused output channels (same as in your current code)
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
      buffer.clear(i, 0, buffer.getNumSamples());
  }

  if (!juce::JUCEApplicationBase::isStandaloneApp())
  {
    if (auto* playhead = getPlayHead())
    {
        if (auto positionInfo = playhead->getPosition())
        {          
          if (positionInfo->getBpm().hasValue()) {
            nowBpm = static_cast<float>(*(positionInfo->getBpm()));

            if (nowBpm != currentBPM)
            {
              currentBPM = nowBpm;
              rack.setBPM(currentBPM);
            }
          }
        }
    }
  }

  // Create AudioBlock from the AudioBuffer for processing
  juce::dsp::AudioBlock<float> block(buffer);

  // Process the block with the RackProcessor
  rack.process(block);

}

//==============================================================================
bool DerangerAudioProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *DerangerAudioProcessor::createEditor() {
  return new DerangerAudioProcessorEditor(*this);
}

RackProcessor& DerangerAudioProcessor::getRack() { return this->rack; }

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new DerangerAudioProcessor();
}
