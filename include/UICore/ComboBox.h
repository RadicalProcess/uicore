#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

namespace rp::uicore
{
    class ComboBox : public juce::ComboBox
    {
    public:
        explicit ComboBox(const std::string& name);

        ~ComboBox() override;

        // Invoked just before the drop-down popup is shown, giving the host a
        // chance to refresh which items are enabled.
        std::function<void()> onBeforePopup;

        void mouseDown(const juce::MouseEvent& event) override;

    private:

        std::unique_ptr<juce::LookAndFeel> lf_;
    };

}
