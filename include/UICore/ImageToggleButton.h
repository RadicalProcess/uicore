#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore
{
    class ImageToggleButton : public juce::ToggleButton
    {
    public:
        ImageToggleButton(const juce::Image& imageOff, const juce::Image& imageOn);

        ~ImageToggleButton() override = default;
    private:
        void paintButton(juce::Graphics& g, bool /*isMouseOverButton*/, bool /*isButtonDown*/) override;

        juce::Image imageOff_;
        juce::Image imageOn_;
    };
}
