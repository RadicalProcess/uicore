#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <UICore/RotarySlider.h>
#include <UICore/StraightSlider.h>
#include <UICore/StepSlider.h>
#include <UICore/Label.h>
#include <UICore/TextField.h>

namespace rp::uitest
{
    class MainComponent : public juce::Component
    {
    public:
        MainComponent();

        void paint(juce::Graphics &) override;

        void resized() override;

    private:

        uicore::Label label_;
        uicore::StandardRotarySlider rSlider_;
        uicore::VerticalSlider vSlider_;
        uicore::StepSlider sSlider_;
        uicore::CenterDefaultRotarySlider crSlider_;
        uicore::HorizontalSlider hSlider_;
        uicore::DecibelRotarySlider dbSlider_;
        uicore::TextField textField_;
        uicore::VerticalRangeSlider vrSlider_;
        juce::Image logoImage_;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
    };

}
