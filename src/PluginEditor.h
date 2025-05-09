#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ui/LabelWithBackground.h"
#include "ui/ToggleLookAndFeel.h"
#include "ui/VisualizerComponent.h"
#include "ui/DynamicLookAndFeel.h"

//==============================================================================
/**
 */
class DerangerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       private juce::Timer {
 public:
 DerangerAudioProcessorEditor(DerangerAudioProcessor &);
  ~DerangerAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;
  void timerCallback() override;
  void takeSnapshotOfGUI (juce::Component* comp);

  DelayProcessor*   findDelayProcessor();
  ReverbProcessor*  findReverbProcessor();
  FlangerProcessor* findFlangerProcessor(); // TODO: Generalize these functions

 private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  DerangerAudioProcessor &audioProcessor;

  juce::ToggleButton  isParallelButton;
  juce::ToggleButton  randomizeButton;
  juce::ToggleButton  stretchButton;
  juce::Slider        stretchSemitoneKnob;

  // Reverb Sliders
  juce::Slider        reverbRoomSizeSlider, reverbWetSlider, reverbDampingSlider;
  LabelWithBackground reverbRoomSizeLabel,  reverbWetLabel,  reverbDampingLabel;
  juce::ToggleButton  reverbRoomSizeToggle, reverbWetToggle, reverbDampingToggle;
  // Delay Sliders
  juce::Slider        delayTimeSlider, delayFeedbackSlider;
  LabelWithBackground delayTimeLabel,  delayFeedbackLabel;
  juce::ToggleButton  delayTimeToggle, delayFeedbackToggle;
  // Flanger Sliders
  juce::Slider        flangerDepthSlider, flangerFeedbackSlider, flangerDelaySlider;
  LabelWithBackground flangerDepthLabel,  flangerFeedbackLabel,  flangerDelayLabel;
  juce::ToggleButton  flangerDepthToggle, flangerFeedbackToggle, flangerDelayToggle;

  juce::TooltipWindow tooltipWindow;
  ToggleLookAndFeel toggleLookAndFeel;
  DynamicLookAndFeel dynamicLookAndFeel;

  juce::Label bpmLabel;
  double _currentBpm = 0.0f;

  // Sliders helpers
  void addAndConfigureSlider(juce::Slider& slider, juce::Label& label, juce::ToggleButton& toggle,
                             const juce::String& name, float min, float max, float initial);

  void updateSliderValues(RackEffect& effect, const std::string& effectName);
  void updateControlsFromParameters();

  juce::GroupComponent sliderContainer {"Sliders" };
  static constexpr int numSliders = 8;
  VisualizerComponent visualizer { audioProcessor };

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DerangerAudioProcessorEditor)
};
