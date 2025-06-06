#include "./PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DerangerAudioProcessor::DerangerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : parameters (*this, nullptr, juce::Identifier("PARAMETERS"),
    {
      std::make_unique<juce::AudioParameterFloat>("delayTime", "Delay Time", 0.05f, 3.0f, 1.0f),
      std::make_unique<juce::AudioParameterFloat>("delayFeedback", "Delay Feedback", 0.0f, 1.0f, 0.7f),
      std::make_unique<juce::AudioParameterFloat>("roomSize", "Room Size", 0.0f, 1.0f, 0.6f),
      std::make_unique<juce::AudioParameterFloat>("wetLevel", "Wet Level", 0.0f, 1.0f, 0.9f),
      std::make_unique<juce::AudioParameterFloat>("damping", "Damping", 0.0f, 1.0f, 0.5f),
      std::make_unique<juce::AudioParameterFloat>("flangerFeedback", "Flanger Feedback", 0.0f, 1.0f, 0.66f),
      std::make_unique<juce::AudioParameterFloat>("flangerDelay", "Flanger Delay", 1.0f, 20.0f, 10.0f),
      std::make_unique<juce::AudioParameterFloat>("flangerDepth", "Flanger Depth", 0.0f, 1.0f, 0.6f),
      std::make_unique<juce::AudioParameterBool>("isParallel", "Is Parallel", false),
      std::make_unique<juce::AudioParameterBool>("randomize", "Randomize", true),
      std::make_unique<juce::AudioParameterBool>("stretchEnabled", "Stretch Enabled", true),
      std::make_unique<juce::AudioParameterFloat>("stretchSemitones", "Stretch Semitones", -12.0f, 12.0f, -5.0f),
    }),
      AudioProcessor(
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

  /* NOTE: Currently, all the effects (reverb, delay, flanger)
           must always be added to the rack: */
  rack.addFlanger(parameters);
  rack.addDelay  (parameters);
  rack.addReverb (parameters);
  rack.addEnd();

  rack.printTree(&rack.getRoot(), 0);

  initializeParameters(parameters);
}

DerangerAudioProcessor::~DerangerAudioProcessor() {}

//======= States and Parameters ================================================

void DerangerAudioProcessor::initializeParameters(juce::AudioProcessorValueTreeState& params, bool updateEffects)
{
    randomizeParam = params.getRawParameterValue("randomize");
    stretchEnabledParam = params.getRawParameterValue("stretchEnabled");
    stretchSemitonesParam = params.getRawParameterValue("stretchSemitones");
    isParallelParam = params.getRawParameterValue("isParallel");

    delayTimeParam = params.getRawParameterValue("delayTime");
    delayFeedbackParam = params.getRawParameterValue("delayFeedback");

    roomSizeParam = params.getRawParameterValue("roomSize");
    wetLevelParam = params.getRawParameterValue("wetLevel");
    dampingParam = params.getRawParameterValue("damping");

    flangerFeedbackParam = params.getRawParameterValue("flangerFeedback");
    flangerDelayParam = params.getRawParameterValue("flangerDelay");
    flangerDepthParam = params.getRawParameterValue("flangerDepth");

    if (updateEffects) {
      rack.setStretchSemitones(*stretchSemitonesParam);
      rack.setStretchEnabled(*stretchEnabledParam);
      rack.getRoot().setParallel(*isParallelParam);
      rack.setRandomize(*randomizeParam);

      dynamic_cast<DelayProcessor*>(rack.findProcessor("Delay"))->setDelayTime(*delayTimeParam * (float)_sampleRate);
      dynamic_cast<DelayProcessor*>(rack.findProcessor("Delay"))->setFeedback(*delayFeedbackParam);

      auto params = dynamic_cast<ReverbProcessor*>(rack.findProcessor("Reverb"))->getParameters();
      params.roomSize = *roomSizeParam;
      params.damping = *dampingParam;
      params.wetLevel = *wetLevelParam;
      dynamic_cast<ReverbProcessor*>(rack.findProcessor("Reverb"))->setParameters(params);

      dynamic_cast<FlangerProcessor*>(rack.findProcessor("Flanger"))->setFeedback(*flangerFeedbackParam);
      dynamic_cast<FlangerProcessor*>(rack.findProcessor("Flanger"))->setDelay(*flangerDelayParam);
      dynamic_cast<FlangerProcessor*>(rack.findProcessor("Flanger"))->setLFODepth(*flangerDepthParam);
    }
}

void DerangerAudioProcessor::applyEffectParamChanges(const std::map<std::string, float>& paramMap) const
{
    for (const auto& [id, value] : paramMap)
    {
        juce::Identifier paramId(id);
        if (auto* param = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter(id)))
        {
            param->setValueNotifyingHost(value);
        } else if (auto* param = dynamic_cast<juce::AudioParameterBool*>(parameters.getParameter(id)))
        {
            param->setValueNotifyingHost(value);
        } else {
            printf("!! WARNING: Param '%s' not found or not float/bool!\n", id.c_str());
        }
    }
}

void DerangerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
  for (auto param : parameters.state) // TODO (amp1ee): remove this w/a:
  {
      auto param_name = param.getProperty("id").toString().toRawUTF8();
      if (std::strcmp(param_name, "delayTime") == 0)
        param.setProperty("value", dynamic_cast<DelayProcessor*>(rack.findProcessor("Delay"))->getTargetDelayTime()/_sampleRate, nullptr);
      else if (std::strcmp(param_name, "flangerDelay") == 0)
        param.setProperty("value", dynamic_cast<FlangerProcessor*>(rack.findProcessor("Flanger"))->getDelay(), nullptr);
      else if (std::strcmp(param_name, "stretchSemitones") == 0)
        param.setProperty("value", rack.getStretchSemitones(), nullptr);
  }

  // Saving the state to XML
  std::unique_ptr<juce::XmlElement> xml (parameters.state.createXml());
  copyXmlToBinary (*xml, destData);
}

void DerangerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
  // Restore the state from XML
  std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
  if (xmlState != nullptr) {
    if (xmlState->hasTagName (parameters.state.getType())) {
        parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
        if (onStateChanged) {
            onStateChanged();
        }
        initializeParameters(parameters, true);
    }
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

  auto numSamples = buffer.getNumSamples();
  sum = 0.0f;
  for (int ch = 0; ch < totalNumOutputChannels; ++ch)
  {
      auto* data = buffer.getReadPointer(ch);
      for (int i = 0; i < numSamples; ++i)
          sum += data[i] * data[i];
  }

  rms = std::sqrt(sum / (numSamples * totalNumOutputChannels));
  currentRMSLevel.store(rms);
  currentInstantLevel.store(buffer.getMagnitude(0, numSamples));

  if (buffer.getNumChannels() >= 2)
  {
      auto* left = buffer.getReadPointer(0);
      auto* right = buffer.getReadPointer(1);
      auto numSamples = buffer.getNumSamples();

      for (int i = 0; i < numSamples; ++i)
      {
          leftPeak  = std::max(leftPeak,  std::abs(left[i]));
          rightPeak = std::max(rightPeak, std::abs(right[i]));
      }

      float width = std::abs(leftPeak - rightPeak);
      currentStereoWidth.store(width);
  } else { currentStereoWidth.store(0.0f); }
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
