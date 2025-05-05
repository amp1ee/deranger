#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
DerangerAudioProcessorEditor::DerangerAudioProcessorEditor(
    DerangerAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {


  setSize(420, 690);

  svgimg = juce::Drawable::createFromImageData(BinaryData::amplee_svg,
                                                 BinaryData::amplee_svgSize);

  addAndMakeVisible(sliderContainer);

  auto *rev = findReverbProcessor();
  addAndConfigureSlider(reverbRoomSizeSlider, reverbRoomSizeLabel, reverbRoomSizeToggle, "RV Size", 0.0f, 1.0f, rev->getParameters().roomSize);
  addAndConfigureSlider(reverbWetSlider, reverbWetLabel, reverbWetToggle, "RV Wet", 0.0f, 1.0f, rev->getParameters().wetLevel);
  addAndConfigureSlider(reverbDampingSlider, reverbDampingLabel, reverbDampingToggle, "RV Damping", 0.0f, 1.0f, rev->getParameters().damping);
  
  auto *del = findDelayProcessor();
  addAndConfigureSlider(delayTimeSlider, delayTimeLabel, delayTimeToggle, "DL Time", 0.05, 3, del->getDelayTime()/audioProcessor.getSampleRate());
  addAndConfigureSlider(delayFeedbackSlider, delayFeedbackLabel, delayFeedbackToggle, "DL Feedback", 0.0f, 1.0f, del->getFeedback());

  auto *flg = findFlangerProcessor();
  addAndConfigureSlider(flangerDelaySlider, flangerDelayLabel, flangerDelayToggle, "FL Time", 1.0f, 20.0f, flg->getDelay());
  addAndConfigureSlider(flangerDepthSlider, flangerDepthLabel, flangerDepthToggle, "FL Depth", 0.0f, 1.0f, flg->getLFODepth());
  addAndConfigureSlider(flangerFeedbackSlider, flangerFeedbackLabel, flangerFeedbackToggle, "FL Feedback", 0.0f, 1.0f, flg->getFeedback());

  p.getRack().getRoot().onEffectParamsChanged = [this](RackEffect* effect, const std::string& name)
  {
    if (effect) {
      updateSliderValues(*effect, name);
      this->audioProcessor.applyEffectParamChanges(effect->getParameterMap());
    }
  };
  
  p.onStateChanged = [this]()
  {
      updateControlsFromParameters();
      repaint(); // TODO (amp1ee): Needed?
  };
  
  // === Routing and Random Controls ===
  isParallelButton.setButtonText("||");
  isParallelButton.setToggleState(p.getRack().getRoot().getParallel(), juce::dontSendNotification);
  addAndMakeVisible(isParallelButton);

  randomizeButton.setButtonText("<?>");
  randomizeButton.setToggleState(p.getRack().getRandomize(), juce::dontSendNotification);
  addAndMakeVisible(randomizeButton);

  stretchButton.setButtonText(juce::String::fromUTF8("↑↓"));
  stretchButton.setToggleState(p.getRack().getStretchEnabled(), juce::dontSendNotification);
  addAndMakeVisible(stretchButton);

  // Stretch Semitone Knob
  addAndMakeVisible(stretchSemitoneKnob);
  stretchSemitoneKnob.setSliderStyle(juce::Slider::Rotary);
  stretchSemitoneKnob.setTextBoxStyle(juce::Slider::TextBoxRight, false, 28, 18);
  stretchSemitoneKnob.setRange(-12, 12, 1);
  stretchSemitoneKnob.setValue(audioProcessor.getRack().getStretchSemitones());
  stretchSemitoneKnob.setNumDecimalPlacesToDisplay(0);
  stretchSemitoneKnob.setColour(juce::Slider::thumbColourId, juce::Colours::aqua);
  stretchSemitoneKnob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::aqua);
  stretchSemitoneKnob.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
  stretchSemitoneKnob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
  stretchSemitoneKnob.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::darkslategrey);

  stretchSemitoneKnob.onValueChange = [this]() {
      int semitone = static_cast<int>(stretchSemitoneKnob.getValue());
      audioProcessor.getRack().setStretchSemitones(semitone);
      audioProcessor.applyEffectParamChanges({
        {"stretchSemitones", static_cast<float>(semitone)}
      });
  };

  if (!juce::JUCEApplicationBase::isStandaloneApp())
  {
    bpmLabel.setText("BPM: " + juce::String(p.getCurrentBPM(), 2), juce::dontSendNotification);
    bpmLabel.setColour(juce::Label::textColourId, juce::Colours::antiquewhite);
    bpmLabel.setJustificationType(juce::Justification::centredLeft);
    bpmLabel.attachToComponent(&randomizeButton, false);
    addAndMakeVisible(bpmLabel);
    startTimerHz(10);
  }

  isParallelButton.onStateChange = [this]() {
    bool state = isParallelButton.getToggleState();
    audioProcessor.getRack().getRoot().setParallel(state);
    audioProcessor.applyEffectParamChanges({
      {"isParallel", static_cast<bool>(state)}
    });
  };

  randomizeButton.onStateChange = [this]() {
    audioProcessor.getRack().setRandomize(randomizeButton.getToggleState());
    audioProcessor.applyEffectParamChanges({
      {"randomize", static_cast<bool>(randomizeButton.getToggleState())}
    });
  };

  stretchButton.onStateChange = [this]() {
    bool state = stretchButton.getToggleState();
    audioProcessor.getRack().setStretchEnabled(state);
    stretchSemitoneKnob.setEnabled(state);
    audioProcessor.applyEffectParamChanges({
      {"stretchEnabled", static_cast<bool>(state)}
    });
  };

  // === Reverb Sliders ===
    reverbRoomSizeSlider.onValueChange = [this]() {
      if (auto* reverb = findReverbProcessor())
      {
        auto params = reverb->getParameters();
        params.roomSize = static_cast<float>(reverbRoomSizeSlider.getValue());
        reverb->setParameters(params);
        audioProcessor.applyEffectParamChanges({
            {"roomSize", params.roomSize}
        });
      }
    };

    reverbWetSlider.onValueChange = [this]() {
      if (auto* reverb = findReverbProcessor())
      {
        auto params = reverb->getParameters();
        params.wetLevel = static_cast<float>(reverbWetSlider.getValue());
        reverb->setParameters(params);
        audioProcessor.applyEffectParamChanges({
            {"wetLevel", params.wetLevel}
        });
      }
    };

    reverbDampingSlider.onValueChange = [this]() {
      if (auto* reverb = findReverbProcessor())
      {
        auto params = reverb->getParameters();
        params.damping = static_cast<float>(reverbDampingSlider.getValue());
        reverb->setParameters(params);
        audioProcessor.applyEffectParamChanges({
            {"damping", params.damping}
        });
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
        delay->setDelayTime(static_cast<float>(delayTimeSlider.getValue()
                                    * audioProcessor.getSampleRate()));
        audioProcessor.applyEffectParamChanges({
            {"delayTime", delayTimeSlider.getValue()}
        });
      }
    };
    delayFeedbackSlider.onValueChange = [this]() {
      if (auto* delay = findDelayProcessor())
      {
        delay->setFeedback(static_cast<float>(delayFeedbackSlider.getValue()));
        audioProcessor.applyEffectParamChanges({
            {"delayFeedback", delay->getFeedback()}
        });
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
      if (auto* flanger = findFlangerProcessor()) {
        flanger->setDelay(static_cast<float>(flangerDelaySlider.getValue()));
        audioProcessor.applyEffectParamChanges({
            {"flangerDelay", flanger->getDelay()}
        });
      }
    };

    flangerDepthSlider.onValueChange = [this]() {
      if (auto* flanger = findFlangerProcessor()) {
        flanger->setLFODepth(static_cast<float>(flangerDepthSlider.getValue()));
        audioProcessor.applyEffectParamChanges({
            {"flangerDepth", flanger->getLFODepth()}
        });
      }
    };

    flangerFeedbackSlider.onValueChange = [this]() {
      if (auto* flanger = findFlangerProcessor()) {
        flanger->setFeedback(static_cast<float>(flangerFeedbackSlider.getValue()));
        audioProcessor.applyEffectParamChanges({
            {"flangerFeedback", flanger->getFeedback()}
        });
      }
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

void DerangerAudioProcessorEditor::updateControlsFromParameters()
{
    auto& params = audioProcessor.parameters;

    // Use helper function to avoid invalid casts
    auto getFloatParam = [&](const juce::String& id) -> float
    {
        if (auto* param = dynamic_cast<juce::AudioParameterFloat*>(params.getParameter(id)))
            return param->get();
        return 0.0f;
    };

    auto getBoolParam = [&](const juce::String& id) -> bool
    {
        if (auto* param = dynamic_cast<juce::AudioParameterBool*>(params.getParameter(id)))
            return param->get();
        return false;
    };

    auto nomsg = juce::dontSendNotification;
    delayTimeSlider.setValue(getFloatParam("delayTime"), nomsg);
    delayFeedbackSlider.setValue(getFloatParam("delayFeedback"), nomsg);
    reverbRoomSizeSlider.setValue(getFloatParam("roomSize"), nomsg);
    reverbWetSlider.setValue(getFloatParam("wetLevel"), nomsg);
    reverbDampingSlider.setValue(getFloatParam("damping"), nomsg);
    flangerDelaySlider.setValue(getFloatParam("flangerDelay"), nomsg);
    flangerDepthSlider.setValue(getFloatParam("flangerDepth"), nomsg);
    flangerFeedbackSlider.setValue(getFloatParam("flangerFeedback"), nomsg);

    isParallelButton.setToggleState(getBoolParam("isParallel"), nomsg);
    randomizeButton.setToggleState(getBoolParam("randomize"), nomsg);
    stretchButton.setToggleState(getBoolParam("stretchEnabled"), nomsg);
    stretchSemitoneKnob.setValue(getFloatParam("stretchSemitones"), nomsg);
    stretchSemitoneKnob.setEnabled(getBoolParam("stretchEnabled"));
}

void DerangerAudioProcessorEditor::timerCallback()
{
    float bpm = audioProcessor.getCurrentBPM();  // Atomic safe read

    bpmLabel.setText("BPM: " + juce::String(bpm, 2), juce::dontSendNotification);
}

DerangerAudioProcessorEditor::~DerangerAudioProcessorEditor() {
    audioProcessor.getRack().getRoot().onEffectParamsChanged = nullptr;
    svgimg = nullptr;
    reverbRoomSizeToggle.setLookAndFeel(nullptr);
    reverbWetToggle.setLookAndFeel(nullptr);
    reverbDampingToggle.setLookAndFeel(nullptr);
    delayTimeToggle.setLookAndFeel(nullptr);
    delayFeedbackToggle.setLookAndFeel(nullptr);
    flangerDelayToggle.setLookAndFeel(nullptr);
    flangerDepthToggle.setLookAndFeel(nullptr);
    flangerFeedbackToggle.setLookAndFeel(nullptr);
    isParallelButton.setLookAndFeel(nullptr);
    randomizeButton.setLookAndFeel(nullptr);
    stretchButton.setLookAndFeel(nullptr);
}

//==============================================================================
void DerangerAudioProcessorEditor::paint(juce::Graphics &g) {
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
void DerangerAudioProcessorEditor::resized() {
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
  auto left = buttonRow.removeFromLeft(buttonRow.getWidth() / 3);
  isParallelButton.setBounds(left.reduced(4));
 
  auto middle = buttonRow.removeFromLeft(buttonRow.getWidth() / 2);
  randomizeButton.setBounds(middle.reduced(4));

  auto stretchGroup = buttonRow;
  auto stretchToggleBounds = stretchGroup.removeFromLeft(rowHeight + 8); // toggle square
  stretchButton.setBounds(stretchToggleBounds.reduced(4));
  
  int knobSize = 75;
  auto knobArea = stretchGroup.removeFromLeft(rowHeight + 16); // knob size
  stretchSemitoneKnob.setBounds(knobArea.withSizeKeepingCentre(knobSize, knobSize));

  static bool snapshotTaken = false;
  if (!snapshotTaken) {
      juce::MessageManager::callAsync([this]() {
        takeSnapshotOfGUI(this);  // Capture the snapshot of the entire editor component
      });
      snapshotTaken = true;
  }

  if (_currentBpm != audioProcessor.getCurrentBPM()) {
    _currentBpm = audioProcessor.getCurrentBPM();
    bpmLabel.setText("BPM: " + juce::String(_currentBpm, 2), 
                          juce::dontSendNotification);
  }
}

void DerangerAudioProcessorEditor::takeSnapshotOfGUI (juce::Component* comp)
{
    juce::File projectDir = juce::File::getCurrentWorkingDirectory();
    juce::File shotPng = projectDir.getChildFile("snapshot.png");
    // Create the snapshot of the component
    juce::Image snapShot = comp->createComponentSnapshot(comp->getLocalBounds());

    int shotWidth = comp->getWidth();
    int shotHeight = (snapShot.getHeight() * shotWidth) / snapShot.getWidth();
    juce::Image shot = snapShot.rescaled(shotWidth, shotHeight);

    // Write the image to file
    if (juce::ImageFileFormat* format = juce::ImageFileFormat::findImageFormatForFileExtension(shotPng))
    {
        juce::FileOutputStream out(shotPng);
        if (out.openedOk())
        {
            // Write the resized thumbnail to the output stream
            format->writeImageToStream(shot, out);
        }
    }
}

void DerangerAudioProcessorEditor::addAndConfigureSlider(juce::Slider& slider, juce::Label& label, juce::ToggleButton& toggle,
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

void DerangerAudioProcessorEditor::updateSliderValues(RackEffect& effect, std::string effectName)
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

    delayTimeSlider.setValue(del->getDelayTime() / audioProcessor.getSampleRate(), nomsg);
    delayFeedbackSlider.setValue(del->getFeedback(), nomsg);

  } else if (effectName == "Flanger") {
    auto *flg = dynamic_cast<FlangerProcessor *>(&effect);

    flangerDelaySlider.setValue(flg->getDelay(), nomsg);
    flangerDepthSlider.setValue(flg->getLFODepth(), nomsg);
    flangerFeedbackSlider.setValue(flg->getFeedback(), nomsg);
  }
}

ReverbProcessor* DerangerAudioProcessorEditor::findReverbProcessor()
{
    return dynamic_cast<ReverbProcessor*>(audioProcessor.getRack().findProcessor("Reverb"));
}

DelayProcessor* DerangerAudioProcessorEditor::findDelayProcessor()
{
    return dynamic_cast<DelayProcessor*>(audioProcessor.getRack().findProcessor("Delay"));
}

FlangerProcessor* DerangerAudioProcessorEditor::findFlangerProcessor()
{
    return dynamic_cast<FlangerProcessor*>(audioProcessor.getRack().findProcessor("Flanger"));
}
