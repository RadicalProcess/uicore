#include "StepSlider.h"
#include "Style.h"
#include "Font.h"

namespace rp::uicore
{
    class StepSliderLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        explicit StepSliderLookAndFeel(size_t steps)
                : steps_(steps){

        }

        void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                               const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
        {
            const auto radius = static_cast<float>(juce::jmin(width / 2, height / 2)) - 4.0f;
            const auto centerX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
            const auto centerY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
            const auto angleRange = rotaryEndAngle - rotaryStartAngle;
            const auto stepAngle = angleRange / static_cast<float>(steps_-1);
            const auto angle = rotaryStartAngle + sliderPos * angleRange;
            const auto gap = 0.1f;

            {
                auto value = static_cast<int>(slider.getValue() - slider.getMinimum());
                for (auto i = static_cast<size_t>(0); i < (steps_-1); ++i)
                {
                    auto p = juce::Path();
                    const auto start = stepAngle * static_cast<float>(i) + rotaryStartAngle;
                    const auto end = start + stepAngle;
                    p.addCentredArc(centerX, centerY, radius, radius, 0.0f, start + gap, end - gap, true);
                    g.setColour(value > i ? styles::foreground : styles::background);
                    g.strokePath(p, styles::strokeType);
                }
            }

            {
                auto p = juce::Point<float>  (centerX + radius * std::cos (angle - juce::MathConstants<float>::halfPi),
                                              centerY + radius * std::sin (angle - juce::MathConstants<float>::halfPi));

                g.setColour (styles::highlight);
                g.fillEllipse (juce::Rectangle<float> (7, 7).withCentre (p));
            }

            const auto rect = juce::Rectangle<float>(centerX - 50, centerY - 10, 100.0f, 20.0f);
            const auto value = static_cast<int>(slider.getValue());

            g.setFont(getRobotoCondensed());
            g.setColour(styles::text);
            g.drawText(juce::String(value), rect, juce::Justification::centred, false);
        }

    private:
        size_t steps_;
    };

    StepSlider::StepSlider(const std::string& name, int min, size_t steps)
    : juce::Slider(name)
    , lf_(std::make_unique<StepSliderLookAndFeel>(steps))
    {
        setSliderStyle(juce::Slider::SliderStyle::Rotary);
        setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        setRange(min, static_cast<double>(min + static_cast<int>(steps) - 1 ), 1.0);
        setLookAndFeel(lf_.get());
    }

    StepSlider::~StepSlider()
    {
        setLookAndFeel(nullptr);
    }
}
