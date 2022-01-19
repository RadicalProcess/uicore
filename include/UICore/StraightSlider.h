#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Style.h"

namespace rp::uicore
{
    template <juce::Slider::SliderStyle St>
    class StraightSlider : public juce::Slider
    {
    public:
        StraightSlider(const std::string& name)
        : juce::Slider(name)
        {
            setSliderStyle(St);
            setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            setColour(backgroundColourId, styles::background);
            setColour(trackColourId, styles::foreground);
            setColour(thumbColourId, styles::highlight);
        }
    };

    using VerticalSlider = StraightSlider<juce::Slider::SliderStyle::LinearVertical>;
    using HorizontalSlider = StraightSlider<juce::Slider::SliderStyle::LinearHorizontal>;
    using VerticalRangeSlider = StraightSlider<juce::Slider::SliderStyle::TwoValueVertical>;
}
