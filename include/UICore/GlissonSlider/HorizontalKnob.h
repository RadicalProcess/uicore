#pragma once

#include "Knob.h"

namespace rp::uicore::glisson
{
    class HorizontalKnob : public Knob
    {
    public:
        HorizontalKnob(float initial, float width, float yPosition);

    private:
        float convertToValue(const juce::Point<float>& point) override;

        juce::Point<float> convertToPoint(float value) override;

        const float width_;
        const float yPosition_;
    };
}
