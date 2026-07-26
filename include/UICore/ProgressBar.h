#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore
{
    // A small horizontal bar that fills from left to right to show the progress
    // of a long running task, such as rendering. The progress is a normalised
    // value in the range [0, 1]. The track and fill colours come from the bar's
    // ColourIds so a host can restyle a single bar or all of them.
    class ProgressBar : public juce::Component
    {
    public:
        enum ColourIds
        {
            trackColourId = 0x2002000,
            barColourId = 0x2002001
        };

        explicit ProgressBar(const std::string& name);

        ~ProgressBar() override;

        void setProgress(float progress);

    private:

        void paint(juce::Graphics& g) override;

        float progress_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgressBar)
    };
}
