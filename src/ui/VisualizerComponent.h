#include <JuceHeader.h>
#include "../PluginProcessor.h"

class VisualizerComponent : public juce::AnimatedAppComponent
{
    public:
        VisualizerComponent(DerangerAudioProcessor& p) : audioProcessor(p)
        {
            svgimg = juce::Drawable::createFromImageData(BinaryData::amplee_svg,
                BinaryData::amplee_svgSize);
            setSize (42, 69);
            setFramesPerSecond (25);
        }

        ~VisualizerComponent() override {}

        void paint(juce::Graphics& g) override
        {
            instantLevel = audioProcessor.getInstantLevel();
            rmsLevel = audioProcessor.getRMSLevel();
            stereoWidth = audioProcessor.getStereoWidth();

            g.fillAll(
                getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            svgimg->drawWithin(g, getLocalBounds().reduced(24).toFloat(),
                                juce::Justification::centred, 1.0f);
            g.setColour(juce::Colours::black);

            // Background color based on audio
            g.setColour(juce::Colour::fromFloatRGBA(instantLevel, rmsLevel, stereoWidth, 0.5 - std::abs(instantLevel - stereoWidth)));
            //g.setFillType(juce::Colour::fromFloatRGBA(instantLevel, rmsLevel, stereoWidth, 1.0f - std::abs(instantLevel - stereoWidth)));
            int offset = 112;
            g.fillEllipse(getWidth()/2 - (svgimg->getWidth()-offset)/2, getHeight()/2 - (svgimg->getHeight()-offset)/2, svgimg->getWidth() - offset, svgimg->getHeight() - offset);

            g.setColour(juce::Colour::fromFloatRGBA(rmsLevel, instantLevel, stereoWidth, 0.5 - std::abs(instantLevel - stereoWidth)));
            // Draw centered ellipse
            g.fillEllipse(centerX - ellipseWidth / 2.0f,
                centerX - ellipseWidth / 2.0f,
                ellipseWidth,
                ellipseHeight);
        }

        void update() override
        {
            instantLevel = audioProcessor.getInstantLevel();
            rmsLevel = audioProcessor.getRMSLevel();
            stereoWidth = audioProcessor.getStereoWidth();

            if (rmsLevel < 0.001f && instantLevel < 0.001f)
                return;

        
            // Fixed center
            centerX = getWidth()  / 2.0f;
            centerY = getHeight() / 2.0f;
        
            // Size modulated by audio
            ellipseWidth  = 24.0f * 10.0f * instantLevel;
            ellipseHeight = 24.0f * 10.0f * rmsLevel;
            // You must repaint the components using this colour
            //repaint(); // if visualizer uses it (???)
        }

    private:
        DerangerAudioProcessor& audioProcessor;
        std::unique_ptr<juce::Drawable> svgimg;
        juce::Path path;
        juce::Colour colour;
        juce::Point<float> lastPoint;
        juce::Point<float> pt;

        float instantLevel = 0.0f;
        float rmsLevel = 0.0f;
        float stereoWidth = 0.0f;
        int radius = 21;

        float centerX, centerY;
        float ellipseWidth, ellipseHeight;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VisualizerComponent)
};