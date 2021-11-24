#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore
{

    class Slider : public juce::Slider
    {
    public:
        Slider(const std::string& name);

    private:

        std::unique_ptr<juce::LookAndFeel> lf_;
    };

}