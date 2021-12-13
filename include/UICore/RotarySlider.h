#include <juce_gui_basics/juce_gui_basics.h>

#include "Utils.h"
#include "ColorScheme.h"

namespace rp::uicore
{
    class RotarySliderLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        virtual void drawRotaryBackgroundArc(juce::Graphics& g) = 0;
        virtual void drawRotaryTrackArc(juce::Graphics& g) = 0;
        virtual void drawRotaryThumb(juce::Graphics& g) = 0;
        virtual void drawRotaryLabel(juce::Graphics& g) = 0;

    protected:
        float radius_;
        juce::Point<float> center_;
        juce::Range<float> rotaryRange_;
        float angle_;
        float value_;

    private:
        void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
        {
            radius_ = static_cast<float>(juce::jmin (width / 2, height / 2)) - 4.0f;
            center_.setX(static_cast<float>(x) + static_cast<float>(width)  * 0.5f);
            center_.setY(static_cast<float>(y) + static_cast<float>(height) * 0.5f);
            rotaryRange_.setStart(rotaryStartAngle);
            rotaryRange_.setEnd(rotaryEndAngle);
            angle_ = rotaryStartAngle + sliderPos * rotaryRange_.getLength();
            value_ = static_cast<float>(slider.getValue());

            drawRotaryBackgroundArc(g);
            drawRotaryTrackArc(g);
            drawRotaryThumb(g);
            drawRotaryLabel(g);
        }
    };

    class StandardRotarySliderLookAndFeel : public RotarySliderLookAndFeel
    {
    private:
        void drawRotaryBackgroundArc(juce::Graphics& g) override
        {
            auto p = juce::Path();
            p.addCentredArc(center_.getX(), center_.getY(), radius_, radius_, 0.0f, angle_, rotaryRange_.getEnd(), true);
            g.setColour(colors::background);
            g.strokePath(p, juce::PathStrokeType( 3.5f ));
        }

        void drawRotaryTrackArc(juce::Graphics& g) override
        {
            auto p = juce::Path();
            g.setColour(colors::foreground);
            p.addCentredArc(center_.getX(), center_.getY(), radius_, radius_, 0.0f, rotaryRange_.getStart(), angle_, true);
            g.strokePath(p, juce::PathStrokeType( 3.5f ));
        }

        void drawRotaryThumb(juce::Graphics& g) override
        {
            auto p = juce::Point<float>  (center_.getX() + radius_ * std::cos (angle_ - juce::MathConstants<float>::halfPi),
                                          center_.getY() + radius_ * std::sin (angle_ - juce::MathConstants<float>::halfPi));

            g.setColour (colors::highlight);
            g.fillEllipse (juce::Rectangle<float> (7, 7).withCentre (p));
        }

        void drawRotaryLabel(juce::Graphics& g) override
        {
            const auto rect = juce::Rectangle<float>(center_.getX() - 50, center_.getY() - 10, 100.0f, 20.0f);
            const auto value = reduceNumDecimals(value_, 2);

            g.setColour(colors::text);
            g.drawText(juce::String(value), rect, juce::Justification::centred, false);
        }
    };

    class CenterDefaultRotarySliderLookAndFeel : public RotarySliderLookAndFeel
    {
    private:
        void drawRotaryBackgroundArc(juce::Graphics& g) override
        {
            auto p = juce::Path();
            p.addCentredArc(center_.getX(), center_.getY(), radius_, radius_, 0.0f, rotaryRange_.getStart(), rotaryRange_.getEnd(), true);
            g.setColour(colors::background);
            g.strokePath(p, juce::PathStrokeType( 3.5f ));
        }

        void drawRotaryTrackArc(juce::Graphics& g) override
        {
            auto p = juce::Path();
            const auto centerAngle = rotaryRange_.getLength() / 2.0f + rotaryRange_.getStart();
            g.setColour(colors::foreground);
            p.addCentredArc(center_.getX(), center_.getY(), radius_, radius_, 0.0f, centerAngle, angle_, true);
            g.strokePath(p, juce::PathStrokeType(3.5f));
        }

        void drawRotaryThumb(juce::Graphics& g) override
        {
            auto p = juce::Point<float>  (center_.getX() + radius_ * std::cos(angle_ - juce::MathConstants<float>::halfPi),
                                          center_.getY() + radius_ * std::sin(angle_ - juce::MathConstants<float>::halfPi));

            g.setColour (colors::highlight);
            g.fillEllipse (juce::Rectangle<float> (7, 7).withCentre (p));
        }

        void drawRotaryLabel(juce::Graphics& g) override
        {
            const auto rect = juce::Rectangle<float>(center_.getX() - 50, center_.getX() - 10, 100.0f, 20.0f);
            const auto value = reduceNumDecimals(value_, 2);

            g.setColour(colors::text);
            g.drawText(juce::String(value), rect, juce::Justification::centred, false);
        }
    };

    template <typename T>
    class RotarySlider : public juce::Slider
    {
    public:
        explicit RotarySlider(const std::string& name)
        : juce::Slider(name)
        {
            setSliderStyle(juce::Slider::SliderStyle::Rotary);
            setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            setLookAndFeel(&lookAndFeel_);
        }

    private:
        T lookAndFeel_;
    };

    using StandardRotarySlider = RotarySlider<StandardRotarySliderLookAndFeel>;
    using CenterDefaultRotarySlider = RotarySlider<CenterDefaultRotarySliderLookAndFeel>;
}
