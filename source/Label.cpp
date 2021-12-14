#include "Label.h"
#include "Font.h"
#include "Style.h"

namespace rp::uicore
{
    class LabelLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        LabelLookAndFeel()
        {
            setColour(juce::Label::textColourId, uicore::styles::text);
        }
    };

    Label::Label(const std::string& name, const std::string& text)
    : juce::Label(name, text)
    , lf_(std::make_unique<LabelLookAndFeel>())
    {
        setFont(getRobotoCondensed());
        setLookAndFeel(lf_.get());
        setJustificationType(juce::Justification::Flags::centred);
    }

}
