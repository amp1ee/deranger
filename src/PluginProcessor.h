#pragma once

#include <JuceHeader.h>
#include <atomic>
#include "RackProcessor.h"

//==============================================================================
/**
 */
class DerangerAudioProcessor : public juce::AudioProcessor {

 public:
  //==============================================================================
  DerangerAudioProcessor();
  ~DerangerAudioProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  //==============================================================================
  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override;

  //==============================================================================
  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  //==============================================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  RackProcessor& getRack();
  float getCurrentBPM() const { return currentBPM; }

  juce::AudioProcessorValueTreeState parameters;
  std::function<void()> onStateChanged;
  void initializeParameters(juce::AudioProcessorValueTreeState& params);
  void applyEffectParamChanges(const std::map<std::string, float>& paramMap) const;

  std::atomic<float>*randomizeParam;
  std::atomic<float>*stretchEnabledParam;
  std::atomic<float>*stretchSemitonesParam;
  std::atomic<float>*isParallelParam;
  std::atomic<float>*delayTimeParam;
  std::atomic<float>*delayFeedbackParam;
  std::atomic<float>*roomSizeParam;
  std::atomic<float>*wetLevelParam;
  std::atomic<float>*dampingParam;
  std::atomic<float>*flangerFeedbackParam;
  std::atomic<float>*flangerDelayParam;
  std::atomic<float>*flangerDepthParam;

 private:
  RackProcessor rack;

  // BPM Sync
  std::atomic<float> currentBPM = 0.0f;
  float  nowBpm = 0.0f;
  double _sampleRate = 44100.0;
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DerangerAudioProcessor)
};
