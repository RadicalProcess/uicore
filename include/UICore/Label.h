#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore
{

    class Label : public juce::Label
    {
    public:
        Label(const std::string& name);

    private:

        std::unique_ptr<juce::LookAndFeel> lf_;
    };

}
