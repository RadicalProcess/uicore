#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore
{
    // A small horizontal bar that fills from left to right to show the progress
    // of a long running task, such as rendering. The progress is a normalised
    // value in the range [0, 1].
    class ProgressBar : public juce::Component
    {
    public:
        explicit ProgressBar(const std::string& name);

        ~ProgressBar() override;

        void setProgress(double progress);

        double getProgress() const;

        void paint(juce::Graphics& g) override;

    private:

        double progress_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgressBar)
    };
}
