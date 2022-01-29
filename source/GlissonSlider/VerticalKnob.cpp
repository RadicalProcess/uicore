#include <GlissonSlider/VerticalKnob.h>

namespace rp::uicore::glisson
{
    VerticalKnob::VerticalKnob(float initial, float height, float xPosition)
    : Knob(initial)
    , height_(height)
    , xPosition_(xPosition)
    {
        setValue(initial);
    }

    float VerticalKnob::convertToValue(const juce::Point<float>& point)
    {
        const auto yPosition = point.getY();
        return 1.0f - std::clamp((yPosition - 9.f) / (height_ - 18.0f), 0.0f, 1.0f);
    }

    juce::Point<float> VerticalKnob::convertToPoint(float value)
    {
        const auto length = height_ - 18.f;
        return { xPosition_, length * (1.0f- value) + 9.f};
    }
}
