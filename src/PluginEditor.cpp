#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
EffectRackAudioProcessorEditor::EffectRackAudioProcessorEditor(
    EffectRackAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize(400, 300);
  // load Image from BinaryData
  svgimg = juce::Drawable::createFromImageData(BinaryData::jucelogo_svg,
                                               BinaryData::jucelogo_svgSize);
}

EffectRackAudioProcessorEditor::~EffectRackAudioProcessorEditor() {}

//==============================================================================
void EffectRackAudioProcessorEditor::paint(juce::Graphics &g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
  svgimg->drawWithin(g, getLocalBounds().toFloat(),
                     juce::Justification::centred, 1);

  g.setColour(juce::Colours::black);
  g.setFont(30.0f);
  g.drawFittedText("Hello World!", getLocalBounds(),
                   juce::Justification::centred, 1);
}

void EffectRackAudioProcessorEditor::resized() {
  // This is generally where you'll want to lay out the positions of any
  // subcomponents in your editor..
}
