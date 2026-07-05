#include "UICore/ElevationView.h"
#include "UICore/Style.h"

#include <algorithm>

namespace rp::uicore
{

    namespace
    {
        // Radius (in pixels) of the round node markers, and the extra slack added
        // around one when hit testing so it is comfortable to grab.
        const auto nodeRadius_ = 5.0f;
        const auto nodeHitMargin_ = 4.0f;

        // Thickness of the segmented line, and of the frame and zero lines.
        const auto lineWidth_ = 2.0f;
        const auto frameWidth_ = 1.5f;

        // The playhead is a short line crossing the graph at right angles:
        // playheadHalfLength_ is how far it reaches either side of the graph,
        // playheadThickness_ its stroke width, and playheadTangentStep_ the small
        // step (in pixels) used to estimate the graph tangent at the playhead.
        const auto playheadHalfLength_ = 9.0f;
        const auto playheadThickness_ = 2.5f;
        const auto playheadTangentStep_ = 2.0f;

        // Colour of the drawing-area frame and the zero-elevation line: dim
        // enough to read as a backdrop without competing with the graph.
        const auto frameColour_ = juce::Colour(90, 90, 90);

        // The background the view fills itself with.
        const auto backgroundColour_ = juce::Colour(30, 30, 30);
    }

    ElevationView::ElevationView()
    : drawingAreaWidth_(0.0f)
    , dragIndex_(-1)
    , playheadEnabled_(false)
    , playheadPosition_(0.0f)
    {
        setOpaque(true);
    }

    void ElevationView::setNodes(const std::vector<juce::Point<float>>& positions)
    {
        nodes_.clear();
        nodes_.reserve(positions.size());
        for (const auto& position : positions)
            nodes_.push_back({ std::clamp(position.x, 0.0f, 1.0f), std::clamp(position.y, 0.0f, 1.0f) });

        dragIndex_ = -1;
        repaint();
    }

    std::vector<juce::Point<float>> ElevationView::getNodes() const
    {
        return nodes_;
    }

    void ElevationView::setDrawingAreaWidth(float width)
    {
        drawingAreaWidth_ = std::max(0.0f, width);
        repaint();
    }

    void ElevationView::setPlayheadEnabled(bool enabled)
    {
        playheadEnabled_ = enabled;
        repaint();
    }

    void ElevationView::setPlayheadPosition(float position)
    {
        playheadPosition_ = std::clamp(position, 0.0f, 1.0f);
        repaint();
    }

    void ElevationView::paint(juce::Graphics& g)
    {
        g.fillAll(backgroundColour_);

        const auto area = drawingArea();

        // The drawing-area frame and the zero-elevation line across its middle.
        g.setColour(frameColour_);
        g.drawRect(area, frameWidth_);
        g.drawLine(area.getX(), area.getCentreY(), area.getRight(), area.getCentreY(), frameWidth_);

        if (nodes_.empty())
            return;

        // The straight segments joining the nodes in order, built once and
        // reused for the playhead.
        juce::Path path;
        const auto hasGraph = nodes_.size() >= 2;
        if (hasGraph)
        {
            path.startNewSubPath(nodePixel(0));
            for (auto i = 1; i < static_cast<int>(nodes_.size()); ++i)
                path.lineTo(nodePixel(i));

            g.setColour(styles::foreground);
            g.strokePath(path, juce::PathStrokeType(lineWidth_));
        }

        // The node markers: the node being dragged is a filled highlight disc,
        // the others hollow foreground rings.
        for (auto i = 0; i < static_cast<int>(nodes_.size()); ++i)
        {
            const auto centre = nodePixel(i);
            const auto bounds = juce::Rectangle<float>(centre.x - nodeRadius_, centre.y - nodeRadius_, nodeRadius_ * 2.0f, nodeRadius_ * 2.0f);

            if (i == dragIndex_)
            {
                g.setColour(styles::highlight);
                g.fillEllipse(bounds);
            }
            else
            {
                g.setColour(backgroundColour_);
                g.fillEllipse(bounds);
                g.setColour(styles::foreground);
                g.drawEllipse(bounds, frameWidth_);
            }
        }

        // The playhead riding along the graph, drawn on top of everything so the
        // playback position stays visible. It is a short line crossing the graph
        // perpendicular to its tangent, in the highlight colour, to match the
        // TrajectoryView playhead.
        if (hasGraph && playheadEnabled_)
        {
            const auto totalLength = path.getLength();
            const auto distance = playheadPosition_ * totalLength;
            const auto centre = path.getPointAlongPath(distance);

            // Estimate the tangent from points either side of the playhead. A
            // kink in the graph can make a small step degenerate (both points
            // coincide), so widen the step until it yields a direction; this
            // keeps the crossing line perpendicular everywhere. Only a
            // zero-length graph leaves the fallback vertical normal in place.
            auto normal = juce::Point<float>(0.0f, 1.0f);
            for (auto step = playheadTangentStep_; step <= totalLength; step *= 2.0f)
            {
                const auto behind = path.getPointAlongPath(std::max(0.0f, distance - step));
                const auto ahead = path.getPointAlongPath(std::min(totalLength, distance + step));
                const auto tangent = ahead - behind;
                const auto tangentLength = tangent.getDistanceFromOrigin();
                if (tangentLength > 0.0f)
                {
                    normal = juce::Point<float>(-tangent.y, tangent.x) / tangentLength;
                    break;
                }
            }

            g.setColour(styles::highlight);
            g.drawLine(juce::Line<float>(centre - normal * playheadHalfLength_, centre + normal * playheadHalfLength_), playheadThickness_);
        }
    }

    void ElevationView::resized()
    {
        repaint();
    }

    void ElevationView::mouseDown(const juce::MouseEvent& event)
    {
        // Nodes can only be grabbed, never created: clicking empty space does
        // nothing.
        dragIndex_ = nodeAt(event.position);
        if (dragIndex_ >= 0)
            repaint();
    }

    void ElevationView::mouseDrag(const juce::MouseEvent& event)
    {
        if (dragIndex_ < 0)
            return;

        // A node moves vertically only; its horizontal position is fixed. Map
        // the pointer's y into the drawing area and clamp to 0..1.
        const auto area = drawingArea();
        const auto height = area.getHeight();
        const auto value = (height > 0.0f) ? (event.position.y - area.getY()) / height : 0.0f;
        nodes_[static_cast<size_t>(dragIndex_)].y = std::clamp(value, 0.0f, 1.0f);

        notifyChange();
        repaint();
    }

    void ElevationView::mouseUp(const juce::MouseEvent&)
    {
        if (dragIndex_ < 0)
            return;

        dragIndex_ = -1;
        repaint();
    }

    juce::Rectangle<float> ElevationView::drawingArea() const
    {
        const auto bounds = getLocalBounds().toFloat().reduced(nodeRadius_ + 1.0f);

        // A width of 0 means "not set yet": fall back to the full available
        // width. Otherwise use the configured width, clamped to what fits.
        const auto width = (drawingAreaWidth_ > 0.0f) ? std::min(drawingAreaWidth_, bounds.getWidth()) : bounds.getWidth();

        return juce::Rectangle<float>(width, bounds.getHeight()).withCentre(bounds.getCentre());
    }

    juce::Point<float> ElevationView::nodePixel(int index) const
    {
        const auto area = drawingArea();
        const auto& node = nodes_[static_cast<size_t>(index)];

        return { area.getX() + node.x * area.getWidth(), area.getY() + node.y * area.getHeight() };
    }

    int ElevationView::nodeAt(juce::Point<float> point) const
    {
        const auto reach = nodeRadius_ + nodeHitMargin_;

        for (auto i = 0; i < static_cast<int>(nodes_.size()); ++i)
        {
            if (point.getDistanceFrom(nodePixel(i)) <= reach)
                return i;
        }

        return -1;
    }

    void ElevationView::notifyChange() const
    {
        if (onChange)
            onChange();
    }

}
