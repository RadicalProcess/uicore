#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore
{
    // Placeholder for a gesture preview: a flat gray rectangle that draws no
    // content yet. It stands in for a future waveform/motion thumbnail so the
    // GestureView layout can be exercised.
    class ThumbnailComponent : public juce::Component
    {
    public:
        ThumbnailComponent();

        void paint(juce::Graphics &) override;

    private:

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThumbnailComponent)
    };

}
