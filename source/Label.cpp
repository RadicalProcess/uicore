#include "Label.h"

namespace rp::uicore
{
    class LabelLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        LabelLookAndFeel()
        {
            setColour(juce::Label::textColourId , juce::Colours::white);
        }
    };

    Label::Label(const std::string& name)
    : juce::Label(juce::String(name.c_str()), juce::String(name.c_str()))
    , lf_(std::make_unique<LabelLookAndFeel>())
    {
        setLookAndFeel(lf_.get());
    }

}
