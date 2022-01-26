#include "GlissonSlider/HorizontalKnob.h"

namespace rp::uicore::glisson
{
    HorizontalKnob::HorizontalKnob(float initial, float width, float yPosition)
    : Knob(initial)
    , width_(width)
    , yPosition_(yPosition)
    {
        setValue(initial);
    }

    float HorizontalKnob::convertToValue(const juce::Point<float>& point)
    {
        const auto xPosition = point.getX();
        return std::clamp((xPosition - 9.f) / (width_ - 18.0f), 0.0f, 1.0f);
    }

    juce::Point<float> HorizontalKnob::convertToPoint(float value)
    {
        return { (width_ - 18.f) * value + 9.f, yPosition_};
    }
}
