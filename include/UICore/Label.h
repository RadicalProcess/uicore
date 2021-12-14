#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore
{

    class Label : public juce::Label
    {
    public:
        Label(const std::string& name, const std::string& text);

    private:

        std::unique_ptr<juce::LookAndFeel> lf_;
    };

}
