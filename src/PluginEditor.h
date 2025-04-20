#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LabelWithBackground.h"

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

  juce::ToggleButton  isParallelButton;

  // Reverb Sliders
  juce::Slider        reverbRoomSizeSlider, reverbWetSlider, reverbDampingSlider;
  LabelWithBackground reverbRoomSizeLabel,  reverbWetLabel,  reverbDampingLabel;
  // Delay Sliders
  juce::Slider        delayTimeSlider, delayFeedbackSlider;
  LabelWithBackground delayTimeLabel,  delayFeedbackLabel;
  // Flanger Sliders
  juce::Slider        flangerDepthSlider, flangerFeedbackSlider, flangerDelaySlider;
  LabelWithBackground flangerDepthLabel, flangerFeedbackLabel, flangerDelayLabel;

  // Sliders helper
  void addAndConfigureSlider(juce::Slider& slider, juce::Label& label,
    const juce::String& name,
    float min, float max, float initial);

  void updateSliderValues(RackEffect& effect, std::string effectName);

  juce::GroupComponent sliderContainer {"Sliders" };
  static constexpr int numSliders = 8;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectRackAudioProcessorEditor)
};
