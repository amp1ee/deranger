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

  DelayProcessor*   findDelayProcessor();
  ReverbProcessor*  findReverbProcessor();
  FlangerProcessor* findFlangerProcessor(); // TODO: Generalize these functions

 private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  EffectRackAudioProcessor &audioProcessor;
  std::unique_ptr<juce::Drawable> svgimg;

  // Reverb Sliders
  juce::Slider reverbRoomSizeSlider, reverbWetSlider, reverbDampingSlider;
  juce::Label  reverbRoomSizeLabel,  reverbWetLabel,  reverbDampingLabel;
  // Delay Sliders
  juce::Slider delayTimeSlider, delayFeedbackSlider;
  juce::Label  delayTimeLabel,  delayFeedbackLabel;
  // Flanger Sliders
  juce::Slider flangerDepthSlider, flangerFeedbackSlider, flangerDelaySlider;
  juce::Label flangerDepthLabel, flangerFeedbackLabel, flangerDelayLabel;

  // Sliders helper
  void addAndConfigureSlider(juce::Slider& slider, juce::Label& label,
    const juce::String& name,
    float min, float max, float initial);

  void updateSliderValues(RackEffect& effect, std::string effectName);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectRackAudioProcessorEditor)
};
