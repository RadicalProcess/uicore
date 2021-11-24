#include "Slider.h"
#include "Utils.h"

namespace rp::uicore
{
    class SliderLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        SliderLookAndFeel()
        {
        }

        void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                               const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
       {
            auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
            auto centreX = (float) x + (float) width  * 0.5f;
            auto centreY = (float) y + (float) height * 0.5f;
            auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

            {
                auto p = juce::Path();
                g.setColour(juce::Colours::black);
                p.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
                g.strokePath(p, juce::PathStrokeType(4.0f));
            }

            {
                auto p = juce::Path();
                p.addCentredArc(centreX, centreY, radius, radius, 0.0f, angle, rotaryEndAngle, true);
                g.setColour(juce::Colours::grey);
                g.strokePath(p, juce::PathStrokeType( 4.0f ));
            }

            const auto rect = juce::Rectangle<float>(centreX - 50, centreY - 10, 100.0f, 20.0f);
            const auto value = reduceNumDecimals(slider.getValue(), 2);

            g.drawText(juce::String(value), rect, juce::Justification::centred, false);
        }


    };

    Slider::Slider(const std::string& name)
    : juce::Slider(juce::String(name.c_str()))
    , lf_(std::make_unique<SliderLookAndFeel>())
    {
        setSliderStyle(juce::Slider::SliderStyle::Rotary);
        setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        setColour(textBoxTextColourId ,juce::Colours::black);
        setLookAndFeel(lf_.get());
    }
}
