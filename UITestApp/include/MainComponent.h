#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <UICore/Slider.h>
#include <UICore/Label.h>

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
        uicore::Slider slider_;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent);
    };

}