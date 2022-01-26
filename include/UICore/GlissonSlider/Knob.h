#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore::glisson
{
    class Knob
    {
    public:
        Knob(float initial);

        virtual ~Knob() = default;

        float getValue() const;

        void setValue(float value);

        void setPoint(const juce::Point<float>& point);

        const juce::Point<float>& getPoint() const;

        bool isClose(const juce::Point<float>& point) const;

        bool dragged_;

    private:

        virtual float convertToValue(const juce::Point<float>& point) = 0;

        virtual juce::Point<float> convertToPoint(float value) = 0;

        float value_;
        juce::Point<float> point_;
    };
}
