#include "HorizontalSlider.h"
#include "Utils.h"
#include "ColorScheme.h"

namespace rp::uicore
{
    HorizontalSlider::HorizontalSlider(const std::string& name)
    : juce::Slider(juce::String(name.c_str()))
    {
        setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
        setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        setColour(backgroundColourId, colors::background);
        setColour(trackColourId, colors::foreground);
        setColour(thumbColourId, colors::highlight);
    }
}
