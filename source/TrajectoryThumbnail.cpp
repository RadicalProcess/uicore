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

        // Maps a point from a -1..1 axis to a pixel position spanning the given
        // centre and half-extent.
        float toAxisPixel(float centre, float halfExtent, float normalised)
        {
            return centre + normalised * halfExtent;
        }
    }

    TrajectoryThumbnail::TrajectoryThumbnail()
    {
    }

    void TrajectoryThumbnail::setAnchorData(const std::vector<Anchor>& anchors)
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
        buildPath(anchors_, square, path);

        g.setColour(styles::foreground);
        g.strokePath(path, juce::PathStrokeType(curveWidth_));
    }

    void TrajectoryThumbnail::buildPath(const std::vector<Anchor>& anchors,
                                        const juce::Rectangle<float>& square,
                                        juce::Path& path)
    {
        const auto toPixel = [&square](juce::Point<float> normalised)
        {
            return juce::Point<float>(toAxisPixel(square.getCentreX(), square.getWidth() * 0.5f, normalised.x),
                                      toAxisPixel(square.getCentreY(), square.getHeight() * 0.5f, normalised.y));
        };

        path.startNewSubPath(toPixel(anchors.front().position));

        for (auto i = static_cast<size_t>(1); i < anchors.size(); ++i)
        {
            const auto& previous = anchors[i - 1];
            const auto& current = anchors[i];
            path.cubicTo(toPixel(previous.handleOut), toPixel(current.handleIn), toPixel(current.position));
        }
    }

}
