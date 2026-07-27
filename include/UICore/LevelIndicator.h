#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <string>

namespace rp::uicore
{
    // A mono level meter that fills in proportion to a normalised level in the
    // range [0, 1]. Horizontal meters fill from the left edge; vertical meters
    // fill from the bottom edge upwards. The track and level colours come from
    // the meter's ColourIds so a host can restyle a single meter or all of them.
    class LevelIndicator : public juce::Component
    {
    public:
        enum ColourIds
        {
            trackColourId = 0x2004000,
            levelColourId = 0x2004001
        };

        enum class Orientation
        {
            Horizontal,
            Vertical
        };

        LevelIndicator(const std::string& name, Orientation orientation = Orientation::Vertical);

        ~LevelIndicator() override;

        void setLevel(float level);

    private:
        void paint(juce::Graphics& g) override;

        float level_;
        Orientation orientation_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelIndicator)
    };
}
