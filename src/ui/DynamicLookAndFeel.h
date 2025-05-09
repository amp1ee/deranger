#include <JuceHeader.h>

class DynamicLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void setSliderColour(juce::Colour colour) { sliderColour = colour; }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override
    {
        slider.setColour(Slider::ColourIds::rotarySliderFillColourId, sliderColour);
        slider.setColour(Slider::ColourIds::thumbColourId, sliderColour);

        LookAndFeel_V4::drawRotarySlider(g, x, y, width, height,
                                        sliderPosProportional,
                                        rotaryStartAngle,
                                        rotaryEndAngle,
                                        slider);
    }

    void drawLinearSlider(Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        Slider::SliderStyle style, Slider& slider) override
    {
        slider.setColour(Slider::ColourIds::thumbColourId, sliderColour);
        slider.setColour(Slider::ColourIds::trackColourId, sliderColour);

        LookAndFeel_V4::drawLinearSlider(g, x, y, width, height,
                                            sliderPos, minSliderPos, maxSliderPos,
                                            style, slider);
    }

private:
    juce::Colour sliderColour = juce::Colours::white;
};
