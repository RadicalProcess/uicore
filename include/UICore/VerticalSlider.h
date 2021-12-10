#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore
{
    class VerticalSlider : public juce::Slider
    {
    public:
        VerticalSlider(const std::string& name);

    private:
        std::unique_ptr<juce::LookAndFeel> lf_;
    };
}
