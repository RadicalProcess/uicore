#include "GlissonSlider/Knob.h"

namespace rp::uicore::glisson
{
    Knob::Knob(float initial)
    : value_(initial)
    , dragged_(false)
    {
    }

    float Knob::getValue() const
    {
        return value_;
    }

    void Knob::setValue(float value)
    {
        value_ = value;
        point_ = convertToPoint(value_);
    }

    void Knob::setPoint(const juce::Point<float>& point)
    {
        setValue(convertToValue(point));
    }

    const juce::Point<float>& Knob::getPoint() const
    {
        return point_;
    }

    bool Knob::isClose(const juce::Point<float>& point) const
    {
        return point_.getDistanceFrom(point) < 4.0f;
    }
}
