#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore
{
    class ComboBox : public juce::ComboBox
    {
    public:
        explicit ComboBox(const std::string& name);

        ~ComboBox() override;

    private:

        std::unique_ptr<juce::LookAndFeel> lf_;
    };

}
