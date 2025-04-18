#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 */
class EffectRackAudioProcessorEditor : public juce::AudioProcessorEditor {
 public:
 EffectRackAudioProcessorEditor(EffectRackAudioProcessor &);
  ~EffectRackAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;

 private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  EffectRackAudioProcessor &audioProcessor;
  std::unique_ptr<juce::Drawable> svgimg;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectRackAudioProcessorEditor)
};
