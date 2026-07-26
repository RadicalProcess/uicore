#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <utility>

#include "Utils.h"
#include "Style.h"
#include "Font.h"

namespace rp::uicore
{
    // Draws a rotary slider as a flat dial: a filled, outlined plate with a
    // short pointer tick running from mid-radius out to the rim, and the value
    // (plus an optional unit) centred inside it. There is no track arc and no
    // shading, so the dial sits in a flat-design host without reading as a
    // raised, three-dimensional knob.
    //
    // Every colour comes from the slider's own juce::Slider ColourIds, so a host
    // can restyle a single dial or all of them:
    //   * rotarySliderFillColourId    - the plate,
    //   * rotarySliderOutlineColourId - the rim,
    //   * thumbColourId               - the pointer,
    //   * textBoxTextColourId         - the value and unit text.
    class RotarySliderLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        RotarySliderLookAndFeel(size_t numDecimalDigits, std::string unit)
        : numDecimalDigits_(numDecimalDigits)
        , unit_(std::move(unit)){}

        virtual void drawRotaryPlate(juce::Graphics& g)
        {
            const auto plate = juce::Rectangle<float>(radius_ * 2.0f, radius_ * 2.0f).withCentre(center_);

            g.setColour(colours_.plate);
            g.fillEllipse(plate);
            g.setColour(colours_.rim);
            g.drawEllipse(plate, rimWidth_);
        }

        virtual void drawRotaryPointer(juce::Graphics& g)
        {
            const auto direction = juce::Point<float>(std::cos(angle_ - juce::MathConstants<float>::halfPi),
                                                      std::sin(angle_ - juce::MathConstants<float>::halfPi));

            g.setColour(colours_.pointer);
            g.drawLine(juce::Line<float>(center_ + direction * (radius_ * pointerInnerFraction_),
                                         center_ + direction * radius_),
                       pointerWidth_);
        }

        virtual void drawRotaryLabel(juce::Graphics& g)
        {
            drawValueText(g, juce::String(reduceNumDecimals(value_, numDecimalDigits_)));
        }

    protected:
        // The plate, rim, pointer and text colours resolved from the slider's
        // ColourIds for the dial currently being drawn.
        struct Colours
        {
            juce::Colour plate;
            juce::Colour rim;
            juce::Colour pointer;
            juce::Colour text;
        };

        // Draws the given reading centred in the plate, with the unit appended
        // when there is one.
        void drawValueText(juce::Graphics& g, const juce::String& reading)
        {
            const auto text = unit_.empty() ? reading : reading + " " + juce::String(unit_);
            const auto area = juce::Rectangle<float>(radius_ * 2.0f, radius_ * 2.0f).withCentre(center_);

            g.setFont(getRobotoCondensed().withHeight(valueTextHeight_));
            g.setColour(colours_.text);
            g.drawText(text, area, juce::Justification::centred, false);
        }

        // Fraction of the radius the pointer tick starts at, its stroke width
        // and the rim's, and the height of the value text: fixed proportions
        // that keep a small dial and a large one looking like the same control.
        static constexpr float pointerInnerFraction_ = 0.55f;
        static constexpr float pointerWidth_ = 1.4f;
        static constexpr float rimWidth_ = 1.0f;
        static constexpr float valueTextHeight_ = 11.0f;

        const size_t numDecimalDigits_;
        const std::string unit_;
        float radius_{};
        juce::Point<float> center_;
        juce::Range<float> rotaryRange_;
        float angle_{};
        float value_{};
        Colours colours_;

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
            colours_ = { slider.findColour(juce::Slider::rotarySliderFillColourId),
                         slider.findColour(juce::Slider::rotarySliderOutlineColourId),
                         slider.findColour(juce::Slider::thumbColourId),
                         slider.findColour(juce::Slider::textBoxTextColourId) };

            drawRotaryPlate(g);
            drawRotaryPointer(g);
            drawRotaryLabel(g);
        }
    };

    // A dial whose lowest position reads "-inf" rather than its numeric value,
    // for gains that bottom out at silence.
    class RotarySliderLookAndFeelDecibel : public RotarySliderLookAndFeel
    {
    public:
        RotarySliderLookAndFeelDecibel(size_t numDecimalDigits, std::string&& unit)
        : RotarySliderLookAndFeel(numDecimalDigits, std::move(unit))
        {}

    private:
        void drawRotaryLabel(juce::Graphics& g) override
        {
            const auto atMinimum = std::abs(rotaryRange_.getStart() - angle_) < 0.00001f;
            drawValueText(g, atMinimum ? "-inf" : juce::String(reduceNumDecimals(value_, numDecimalDigits_)));
        }
    };

    template <typename T>
    class RotarySlider : public juce::Slider
    {
    public:
        explicit RotarySlider(const std::string& name, size_t numDecimalDigits_ = 2, std::string unit = "")
        : juce::Slider(name)
        , lookAndFeel_(numDecimalDigits_, std::move(unit))
        {
            setSliderStyle(juce::Slider::SliderStyle::Rotary);
            setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            setLookAndFeel(&lookAndFeel_);
        }

        ~RotarySlider() override
        {
            setLookAndFeel(nullptr);
        }

    private:
        T lookAndFeel_;
    };

    using StandardRotarySlider = RotarySlider<RotarySliderLookAndFeel>;
    using DecibelRotarySlider = RotarySlider<RotarySliderLookAndFeelDecibel>;
}
