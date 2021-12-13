#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore
{
    class HorizontalSlider : public juce::Slider
    {
    public:
        HorizontalSlider(const std::string& name);

    private:
        std::unique_ptr<juce::LookAndFeel> lf_;
    };
}
