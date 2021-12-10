#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore
{
    class RotarySlider : public juce::Slider
    {
    public:
        RotarySlider(const std::string& name);

    private:

        std::unique_ptr<juce::LookAndFeel> lf_;
    };

}
