#pragma once

#include <JuceHeader.h>

class ToggleLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto fontSize = jmin (15.0f, (float) button.getHeight() * 0.75f);
        auto tickWidth = fontSize * 1.1f;

        drawTickBox (g, button, 4.0f, ((float) button.getHeight() - tickWidth) * 0.5f,
                        tickWidth, tickWidth,
                        button.getToggleState(),
                        button.isEnabled(),
                        shouldDrawButtonAsHighlighted,
                        shouldDrawButtonAsDown);

        g.setColour (button.findColour (ToggleButton::textColourId));
        g.setFont (fontSize);

        if (! button.isEnabled())
            g.setOpacity (0.5f);

        g.drawFittedText (button.getButtonText(),
                            button.getLocalBounds().withTrimmedLeft (roundToInt (tickWidth) + 10)
                                                    .withTrimmedRight (2),
                            Justification::centredLeft, 10);
    }

    void drawTickBox (Graphics& g, Component& component,
        float x, float y, float w, float h,
        const bool ticked,
        [[maybe_unused]] const bool isEnabled,
        [[maybe_unused]] const bool shouldDrawButtonAsHighlighted,
        [[maybe_unused]] const bool shouldDrawButtonAsDown) override
    {
        Rectangle<float> tickBounds (x, y, w, h);

        g.setColour (juce::Colours::darkgrey);
        g.drawRoundedRectangle (tickBounds, 4.0f, 1.0f);

        if (ticked)
        {
            g.setColour (component.findColour (ToggleButton::tickColourId));
            auto tick = getTickShape (0.75f);
            g.fillPath (tick, tick.getTransformToScaleToFit (tickBounds.reduced (4, 5).toFloat(), false));
        }
    }
};