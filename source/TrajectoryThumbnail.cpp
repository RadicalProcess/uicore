#include "UICore/TrajectoryThumbnail.h"
#include "UICore/Style.h"

#include <algorithm>

namespace rp::uicore
{
    namespace
    {
        // Matches the background the TrajectoryView editor fills itself with,
        // so the thumbnail reads as a miniature of it.
        const auto backgroundColour_ = juce::Colour(30, 30, 30);

        // Thickness of the mini curve, and the margin kept between the
        // inscribed square and the component edges.
        const auto curveWidth_ = 1.5f;
        const auto margin_ = 2.0f;
    }

    TrajectoryThumbnail::TrajectoryThumbnail()
    {
    }

    void TrajectoryThumbnail::setAnchorData(const std::vector<TrajectoryView::Anchor>& anchors)
    {
        anchors_ = anchors;
        repaint();
    }

    void TrajectoryThumbnail::paint(juce::Graphics &g)
    {
        g.fillAll(backgroundColour_);

        if (anchors_.size() < 2)
            return;

        const auto bounds = getLocalBounds().toFloat().reduced(margin_);
        const auto side = std::min(bounds.getWidth(), bounds.getHeight());
        const auto square = juce::Rectangle<float>(side, side).withCentre(bounds.getCentre());

        juce::Path path;
        TrajectoryView::buildPath(anchors_, square, path);

        g.setColour(styles::foreground);
        g.strokePath(path, juce::PathStrokeType(curveWidth_));
    }

}
