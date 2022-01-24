#include "GlissonSlider.h"

#include <utility>
#include "Style.h"

namespace rp::uicore
{
    GlissonSlider::Knob::Knob(float initial, std::function<juce::Point<float>(float)>  converter)
    : value_(initial)
    , converter_(std::move(converter))
    {
        point_ = converter_(value_);
    }

    void GlissonSlider::Knob::setValue(float value)
    {
        value_ = value;
        point_ = converter_(value_);
    }

    const juce::Point<float>& GlissonSlider::Knob::getPoint() const
    {
        return point_;
    }

    bool GlissonSlider::Knob::isClose(const juce::Point<float>& point) const
    {
        return point_.getDistanceFrom(point) < 3.0f;
    }

    GlissonSlider::GlissonSlider()
    : startPositionKnobA_(0.0f, [this](float value){ return juce::Point<float>(getLocalBounds().getWidth() - 18.f * value + 9.f, 5.f);})
    {}

    void GlissonSlider::paint(juce::Graphics& g)
    {
        const auto localBounds = getLocalBounds();
        auto circleBounds = juce::Rectangle<float>(0.f, 0.f, 7.f, 7.f);

        g.setColour(styles::background);
        g.drawRect(3.f, 3.f, localBounds.getWidth()-6.f, localBounds.getHeight()-6.f, 3.f);

        circleBounds.setCentre(startPositionKnobA_.getPoint());
        g.setColour(styles::highlight);
        g.fillEllipse(circleBounds);
    }

    void GlissonSlider::mouseDown(const juce::MouseEvent& event)
    {
        const auto mousePosition = event.position;
        if(startPositionKnobA_.isClose(mousePosition))
            DBG("engaged");
    }

    void GlissonSlider::mouseDrag(const juce::MouseEvent& event)
    {
        Component::mouseDrag(event);
    }

    void GlissonSlider::mouseUp(const juce::MouseEvent& event)
    {
        Component::mouseUp(event);
    }
}
