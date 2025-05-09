#pragma once

#include <JuceHeader.h>

class LabelWithBackground : public juce::Label
{
public:
    LabelWithBackground() {
            setOpaque(false);
    }

    void paint(juce::Graphics& g) override {
        g.setColour(juce::Colour::fromRGBA(0, 0, 0, 64));

        // Calculating and adding transparent background around label's text:
        ga.addLineOfText(getFont(), getText(), 0.0f, 0.0f);
        float textWidth = ga.getBoundingBox(0, -1, true).getWidth();
        juce::Rectangle<int> textBounds = getLocalBounds()
                                        .withSize(textWidth, getFont().getHeight());
        textBounds.setSize(textBounds.getWidth() + 10, textBounds.getHeight() + 10);

        g.fillRect(textBounds);
        juce::Label::paint(g);

    }

private:
    juce::GlyphArrangement ga;

};