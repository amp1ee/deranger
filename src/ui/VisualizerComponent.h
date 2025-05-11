#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "juce_graphics/juce_graphics.h"

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
            rmsLevel     = audioProcessor.getRMSLevel();
            stereoWidth  = audioProcessor.getStereoWidth();

            g.fillAll(juce::Colour::fromRGBA(33, 33, 42, 255));
            svgimg->drawWithin(g, getLocalBounds().reduced(24).toFloat(),
                                 juce::Justification::centred, 1.0f);

            // // Background color based on audio
            // g.setColour(juce::Colour::fromFloatRGBA(instantLevel, rmsLevel, stereoWidth, 0.7f - std::abs(instantLevel - stereoWidth)));
            // //g.setFillType(juce::Colour::fromFloatRGBA(instantLevel, rmsLevel, stereoWidth, 1.0f - std::abs(instantLevel - stereoWidth)));
            int offset = 112;
            // g.fillEllipse(getWidth()/2-(svgimg->getWidth()-offset)/2, getHeight()/2-(svgimg->getHeight()-offset)/2, svgimg->getWidth()-offset, svgimg->getHeight()-offset);

            // Create wiggly effect by modulating radius with audio levels
            const int numPoints = 100;
            const float twoPi = 2.0f * juce::MathConstants<float>::pi;
            
            juce::Path wigglePath;
            for (int i = 0; i < numPoints; ++i) {
                float angle = (i / (float)numPoints) * twoPi;
                
                // Modulate radius based on audio levels and angle
                float wiggleAmount = 20.0f * (instantLevel + rmsLevel);
                float radiusOffset = wiggleAmount * std::sin(angle * 8 + stereoWidth * 10);
                float modRadius = (svgimg->getWidth()-offset-20.0f)/2 + radiusOffset;
                
                float x = getWidth()/2 + std::cos(angle) * modRadius;
                float y = getHeight()/2 + std::sin(angle) * modRadius;
                
                if (i == 0)
                    wigglePath.startNewSubPath(x, y);
                else
                    wigglePath.lineTo(x, y);
            }
            wigglePath.closeSubPath();

            // Apply rotation transformation
            g.saveState();
            g.addTransform(juce::AffineTransform::rotation(rotationAngle, 
                getWidth() / 2.0f, getHeight() / 2.0f));
            
            // First circle - brighter green
            g.setColour(juce::Colour::fromRGBA(144, 96, 96, 48));
            g.fillPath(wigglePath);
            g.restoreState();

            // Create second wiggly circle rotating in opposite direction
            juce::Path wigglePath2;
            for (int i = 0; i < numPoints; ++i) {
                float angle = (i / (float)numPoints) * twoPi;
                
                // Same modulation as before
                float wiggleAmount = 20.0f * (instantLevel + rmsLevel); 
                float radiusOffset = wiggleAmount * std::sin(angle * 8 + stereoWidth * 10);
                float modRadius = (svgimg->getWidth()-offset-20.0f)/2 + radiusOffset;
                
                float x = getWidth()/2 + std::cos(angle) * modRadius;
                float y = getHeight()/2 + std::sin(angle) * modRadius;
                
                if (i == 0)
                    wigglePath2.startNewSubPath(x, y);
                else
                    wigglePath2.lineTo(x, y);
            }
            wigglePath2.closeSubPath();

            // Apply opposite rotation
            g.saveState();
            g.addTransform(juce::AffineTransform::rotation(-rotationAngle,
                getWidth() / 2.0f, getHeight() / 2.0f));
            
            // Second circle - darker green
            g.setColour(juce::Colour::fromRGBA(72, 96, 56, 48));
            g.fillPath(wigglePath2);
            g.restoreState();

            // g.setColour(juce::Colour::fromFloatRGBA(instantLevel, rmsLevel, stereoWidth, 0.7 - std::abs(instantLevel - stereoWidth)));
            // // Draw centered ellipse
            // g.fillEllipse(centerX - ellipseWidth / 2.0f,
            //     centerY - ellipseHeight / 2.0f,
            //     ellipseWidth,
            //     ellipseHeight);
        }

        void update() override
        {
            instantLevel = audioProcessor.getInstantLevel();
            rmsLevel     = audioProcessor.getRMSLevel();
            stereoWidth  = audioProcessor.getStereoWidth();

            if (rmsLevel < 0.001f && instantLevel < 0.001f)
                return;

            // Update rotation angle based on audio levels
            float rotationSpeed = 0.1f * (instantLevel + rmsLevel);
            rotationAngle += rotationSpeed;
            if (rotationAngle > juce::MathConstants<float>::twoPi)
                rotationAngle -= juce::MathConstants<float>::twoPi;

            // Fixed center
            centerX = getWidth()  / 2.0f;
            centerY = getHeight() / 2.0f;
        
            // Size modulated by audio
            ellipseWidth  = 48.0f * 10.0f * rmsLevel;
            ellipseHeight = 48.0f * 10.0f * rmsLevel;
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
        float rotationAngle = 0.0f;

        float centerX, centerY;
        float ellipseWidth, ellipseHeight;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VisualizerComponent)
};