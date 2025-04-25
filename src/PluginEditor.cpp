#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
EffectRackAudioProcessorEditor::EffectRackAudioProcessorEditor(
    EffectRackAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {


  setSize(400, 600);

  svgimg = juce::Drawable::createFromImageData(BinaryData::jucelogo_svg,
                                               BinaryData::jucelogo_svgSize);

  addAndMakeVisible(sliderContainer);

  auto *rev = findReverbProcessor();
  addAndConfigureSlider(reverbRoomSizeSlider, reverbRoomSizeLabel, reverbRoomSizeToggle, "RV Size", 0.0f, 1.0f, rev->getParameters().roomSize);
  addAndConfigureSlider(reverbWetSlider, reverbWetLabel, reverbWetToggle, "RV Wet", 0.0f, 1.0f, rev->getParameters().wetLevel);
  addAndConfigureSlider(reverbDampingSlider, reverbDampingLabel, reverbDampingToggle, "RV Damping", 0.0f, 1.0f, rev->getParameters().damping);
  
  auto *del = findDelayProcessor();
  addAndConfigureSlider(delayTimeSlider, delayTimeLabel, delayTimeToggle, "DL Time", 3000.0f, 3 * audioProcessor.getSampleRate(), del->getDelayTime());
  addAndConfigureSlider(delayFeedbackSlider, delayFeedbackLabel, delayFeedbackToggle, "DL Feedback", 0.0f, 1.0f, del->getFeedback());

  auto *flg = findFlangerProcessor();
  addAndConfigureSlider(flangerDelaySlider, flangerDelayLabel, flangerDelayToggle, "FL Time", 1.0f, 20.0f, flg->getDelay());
  addAndConfigureSlider(flangerDepthSlider, flangerDepthLabel, flangerDepthToggle, "FL Depth", 0.0f, 1.0f, flg->getLFODepth());
  addAndConfigureSlider(flangerFeedbackSlider, flangerFeedbackLabel, flangerFeedbackToggle, "FL Feedback", 0.0f, 1.0f, flg->getFeedback());

  p.getRack().getRoot().onEffectParamsChanged = [this](RackEffect* effect, const std::string& name)
  {
    if (effect) {
      updateSliderValues(*effect, name);
    }
  };

  // === Routing and Random Controls ===
  isParallelButton.setButtonText("||");
  isParallelButton.setToggleState(p.getRack().getRoot().getParallel(), juce::dontSendNotification);
  addAndMakeVisible(isParallelButton);

  randomizeButton.setButtonText("<?>");
  randomizeButton.setToggleState(p.getRack().getRandomize(), juce::dontSendNotification);
  addAndMakeVisible(randomizeButton);

  isParallelButton.onStateChange = [this]() {
    bool state = isParallelButton.getToggleState();
    audioProcessor.getRack().getRoot().setParallel(state);
  };

  randomizeButton.onClick = [this]() {
    audioProcessor.getRack().setRandomize(randomizeButton.getToggleState());
  };

  // === Reverb Sliders ===
    reverbRoomSizeSlider.onValueChange = [this]() {
      if (auto* reverb = findReverbProcessor())
      {
          auto params = reverb->getParameters();
          params.roomSize = static_cast<float>(reverbRoomSizeSlider.getValue());
          reverb->setParameters(params);
      }
    };

    reverbWetSlider.onValueChange = [this]() {
      if (auto* reverb = findReverbProcessor())
      {
          auto params = reverb->getParameters();
          params.wetLevel = static_cast<float>(reverbWetSlider.getValue());
          reverb->setParameters(params);
      }
    };

    reverbDampingSlider.onValueChange = [this]() {
      if (auto* reverb = findReverbProcessor())
      {
          auto params = reverb->getParameters();
          params.damping = static_cast<float>(reverbDampingSlider.getValue());
          reverb->setParameters(params);
      }
    };

    // === Reverb Toggles ===
    reverbRoomSizeToggle.onClick = [this]() {
      if (auto* reverb = findReverbProcessor())
          reverb->setRoomSizeRandomize(reverbRoomSizeToggle.getToggleState());
    };

    reverbWetToggle.onClick = [this]() {
      if (auto* reverb = findReverbProcessor())
          reverb->setWetLevelRandomize(reverbWetToggle.getToggleState());
    };

    reverbDampingToggle.onClick = [this]() {
      if (auto* reverb = findReverbProcessor())
          reverb->setDampingRandomize(reverbDampingToggle.getToggleState());
    };

    // === Delay Sliders ===
    delayTimeSlider.onValueChange = [this]() {
      if (auto* delay = findDelayProcessor())
      {
          delay->setDelayTime(static_cast<float>(delayTimeSlider.getValue()));
      }
    };
    delayFeedbackSlider.onValueChange = [this]() {
      if (auto* delay = findDelayProcessor())
      {
          delay->setFeedback(static_cast<float>(delayFeedbackSlider.getValue()));
      }
    };

    // === Delay Toggles ===
    delayTimeToggle.onClick = [this]() {
      if (auto* delay = findDelayProcessor())
          delay->setDelayTimeRandomize(delayTimeToggle.getToggleState());
    };
    delayFeedbackToggle.onClick = [this]() {
      if (auto* delay = findDelayProcessor())
          delay->setFeedbackRandomize(delayFeedbackToggle.getToggleState());
    };

    // === Flanger Sliders ===
    flangerDelaySlider.onValueChange = [this]() {
      if (auto* flanger = findFlangerProcessor())
          flanger->setDelay(static_cast<float>(flangerDelaySlider.getValue()));
    };

    flangerDepthSlider.onValueChange = [this]() {
      if (auto* flanger = findFlangerProcessor())
          flanger->setLFODepth(static_cast<float>(flangerDepthSlider.getValue()));
    };

    flangerFeedbackSlider.onValueChange = [this]() {
      if (auto* flanger = findFlangerProcessor())
          flanger->setFeedback(static_cast<float>(flangerFeedbackSlider.getValue()));
    };

    // === Flanger Toggles ===
    flangerDelayToggle.onClick = [this]() {
      if (auto* flanger = findFlangerProcessor())
          flanger->setDelayRandomize(flangerDelayToggle.getToggleState());
    };

    flangerDepthToggle.onClick = [this]() {
      if (auto* flanger = findFlangerProcessor())
          flanger->setDepthRandomize(flangerDepthToggle.getToggleState());
    };

    flangerFeedbackToggle.onClick = [this]() {
      if (auto* flanger = findFlangerProcessor())
          flanger->setFeedbackRandomize(flangerFeedbackToggle.getToggleState());
    };

}

EffectRackAudioProcessorEditor::~EffectRackAudioProcessorEditor() {
    audioProcessor.getRack().getRoot().onEffectParamsChanged = nullptr;
}

//==============================================================================
void EffectRackAudioProcessorEditor::paint(juce::Graphics &g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
  svgimg->drawWithin(g, getLocalBounds().reduced(24).toFloat(),
                    juce::Justification::centred, 1);

  g.setColour(juce::Colours::black);
}


// =====----=================== resized() ======================================== //
//                                                                                 //
void EffectRackAudioProcessorEditor::resized() {
  auto bounds = getLocalBounds().reduced(20);
  int rowHeight = 40;
  int toggleWidth = rowHeight / 2 + 2;
  int spacing = 14;

  sliderContainer.setBounds(10, 10, getWidth() - 20, getHeight() - 20);

  auto row = [&]() {
      return bounds.removeFromTop(rowHeight + spacing).withHeight(rowHeight);
  };

  bounds.removeFromTop(spacing * 3);

  auto sliderBounds = row();
  auto toggleBounds = sliderBounds.removeFromRight(toggleWidth);
  reverbRoomSizeSlider.setBounds(sliderBounds);
  reverbRoomSizeToggle.setBounds(toggleBounds);
  sliderBounds = row();
  toggleBounds = sliderBounds.removeFromRight(toggleWidth);
  reverbWetSlider.setBounds(sliderBounds);
  reverbWetToggle.setBounds(toggleBounds);
  sliderBounds = row();
  toggleBounds = sliderBounds.removeFromRight(toggleWidth);
  reverbDampingSlider.setBounds(sliderBounds);
  reverbDampingToggle.setBounds(toggleBounds);

  bounds.removeFromTop(spacing * 2); // extra space between groups

  sliderBounds = row();
  toggleBounds = sliderBounds.removeFromRight(toggleWidth);
  delayTimeSlider.setBounds(sliderBounds);
  delayTimeToggle.setBounds(toggleBounds);
  sliderBounds = row();
  toggleBounds = sliderBounds.removeFromRight(toggleWidth);
  delayFeedbackSlider.setBounds(sliderBounds);
  delayFeedbackToggle.setBounds(toggleBounds);

  bounds.removeFromTop(spacing * 2);

  sliderBounds = row();
  toggleBounds = sliderBounds.removeFromRight(toggleWidth);
  flangerDelaySlider.setBounds(sliderBounds);
  flangerDelayToggle.setBounds(toggleBounds);
  sliderBounds = row();
  toggleBounds = sliderBounds.removeFromRight(toggleWidth);
  flangerDepthSlider.setBounds(sliderBounds);
  flangerDepthToggle.setBounds(toggleBounds);
  sliderBounds = row();
  toggleBounds = sliderBounds.removeFromRight(toggleWidth);
  flangerFeedbackSlider.setBounds(sliderBounds);
  flangerFeedbackToggle.setBounds(toggleBounds);

  auto buttonRow = row();
  auto left = buttonRow.removeFromLeft(buttonRow.getWidth() / 2);
  isParallelButton.setBounds(left.reduced(4));
  
  auto right = buttonRow; // what's left after removing left
  randomizeButton.setBounds(right.reduced(4));
}

void EffectRackAudioProcessorEditor::addAndConfigureSlider(juce::Slider& slider, juce::Label& label, juce::ToggleButton& toggle,
  const juce::String& name,
  float min, float max, float initial)
  {
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    slider.setRange(min, max);
    slider.setNumDecimalPlacesToDisplay(3);
    slider.setValue(initial);
    slider.setColour(juce::Slider::thumbColourId, juce::Colours::aqua);
    slider.setColour(juce::Slider::trackColourId, juce::Colours::darkcyan);
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::darkslategrey);

    addAndMakeVisible(slider);

    label.setText(name, juce::dontSendNotification);
    label.attachToComponent(&slider, false);
    label.setColour(juce::Label::textColourId, juce::Colours::ghostwhite);

    label.setFont(juce::FontOptions(15, 1)); // 1 = Bold (see juce::Font::FontStyleFlags)
    addAndMakeVisible(label);

    toggle.setColour(juce::ToggleButton::tickColourId, juce::Colours::aqua);
    toggle.setLookAndFeel(&toggleLookAndFeel);
    toggle.setTooltip(name + " Randomize");
    toggle.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(toggle);

}

void EffectRackAudioProcessorEditor::updateSliderValues(RackEffect& effect, std::string effectName)
{
  auto nomsg = juce::dontSendNotification;

  if (effectName == "Reverb") {
    auto *rev = dynamic_cast<ReverbProcessor *>(&effect);
    juce::dsp::Reverb::Parameters par = rev->getParameters();
    
    reverbRoomSizeSlider.setValue(par.roomSize, nomsg);
    reverbDampingSlider.setValue(par.damping, nomsg);
    reverbWetSlider.setValue(par.wetLevel, nomsg);

  } else if (effectName == "Delay") {
    auto *del = dynamic_cast<DelayProcessor *>(&effect);

    //printf("\t\tdel: %f\n", del->getDelayTime());
    delayTimeSlider.setValue(del->getDelayTime(), nomsg);
    delayFeedbackSlider.setValue(del->getFeedback(), nomsg);

  } else if (effectName == "Flanger") {
    auto *flg = dynamic_cast<FlangerProcessor *>(&effect);

    flangerDelaySlider.setValue(flg->getDelay(), nomsg);
    flangerDepthSlider.setValue(flg->getLFODepth(), nomsg);
    flangerFeedbackSlider.setValue(flg->getFeedback(), nomsg);
  }
}

ReverbProcessor* EffectRackAudioProcessorEditor::findReverbProcessor()
{
    for (auto& child : audioProcessor.getRack().getRoot().children)
    {
        if (child->effect && child->effect->getName() == "Reverb")
            return dynamic_cast<ReverbProcessor*>(child->effect.get());
    }
    return nullptr;
}

DelayProcessor* EffectRackAudioProcessorEditor::findDelayProcessor()
{
    for (auto& child : audioProcessor.getRack().getRoot().children)
    {
        if (child->effect && child->effect->getName() == "Delay")
            return dynamic_cast<DelayProcessor*>(child->effect.get());
    }
    return nullptr;
}

FlangerProcessor* EffectRackAudioProcessorEditor::findFlangerProcessor()
{
    for (auto& child : audioProcessor.getRack().getRoot().children)
    {
        if (child->effect && child->effect->getName() == "Flanger")
            return dynamic_cast<FlangerProcessor*>(child->effect.get());
    }
    return nullptr;
}