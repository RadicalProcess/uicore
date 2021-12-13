#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore
{
    class StepSlider : public juce::Slider
    {
    public:
        StepSlider(const std::string& name, int min, size_t steps);

    private:
        std::unique_ptr<juce::LookAndFeel> lf_;
    };
}
