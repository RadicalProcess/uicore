#pragma once

#include "TrajectoryView.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <vector>

namespace rp::uicore
{
    // A miniature, non-interactive preview of a gesture's trajectory: the same
    // cubic bezier curve the TrajectoryView editor draws, scaled into this
    // component's own inscribed square. Only the curve is drawn - no anchors,
    // handles or reference frame - so it stays readable at row-thumbnail size.
    // Feed it the curve with setAnchorData; fewer than two anchors leave a
    // plain background tile.
    class TrajectoryThumbnail : public juce::Component
    {
    public:
        TrajectoryThumbnail();

        void setAnchorData(const std::vector<TrajectoryView::Anchor>& anchors);

        void paint(juce::Graphics &) override;

    private:
        std::vector<TrajectoryView::Anchor> anchors_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryThumbnail)
    };

}
