#pragma once

#include "Knob.h"

namespace rp::uicore::glisson
{
    class VerticalKnob : public Knob
    {
    public:
        VerticalKnob(float initial, float height, float xPosition);

    private:
        float convertToValue(const juce::Point<float>& point) override;

        juce::Point<float> convertToPoint(float value) override;

        const float height_;
        const float xPosition_;
    };
}
