#include "UICore/MotionView.h"
#include "UICore/Style.h"

#include <algorithm>
#include <cmath>

namespace rp::uicore
{

    namespace
    {
        // Half the side length (in pixels) of the square node handles, also used
        // as the inset of the plot area so a handle at an edge stays fully
        // visible.
        const auto nodeHalfSize_ = 6.0f;

        // Extra slack (in pixels) added around a handle when hit testing so the
        // small squares are comfortable to grab.
        const auto nodeHitMargin_ = 4.0f;

        // Thickness of the connecting segments and the handle outlines.
        const auto lineWidth_ = 1.5f;

        // Smallest horizontal gap (normalised) kept between a dragged interior
        // node and its neighbours so the nodes never cross and stay ordered.
        const auto minGap_ = 0.001f;

        // Alpha applied to the foreground colour for the translucent waveform
        // backdrop, kept low so it never competes with the curve drawn on top.
        const auto backdropWaveformAlpha_ = 0.18f;

        // Alpha applied to the highlight colour for the playhead so it reads as
        // part of the faint backdrop rather than the editable curve.
        const auto backdropPlayheadAlpha_ = 0.5f;

        // Height (in pixels) of the numbered reference-line labels.
        const auto referenceLabelHeight_ = 12.0f;

        // Width (in pixels) reserved on the left for a reference-line number,
        // and the gap kept between that number and where its line starts.
        const auto referenceLabelWidth_ = 14.0f;
        const auto referenceLabelGap_ = 4.0f;

        // Colour of the reference lines and their labels: dim enough to read as
        // background guides without competing with the curve.
        const auto referenceColour_ = juce::Colour(90, 90, 90);

        // How close (in pixels) the cursor must be to a segment to hover or grab
        // it for bending.
        const auto segmentHitMargin_ = 5.0f;

        // Number of straight pieces a bent segment is drawn and hit tested with.
        const auto bendSampleCount_ = 32;

        // Depth (normalised) of the bow at the segment midpoint for a full bend
        // of 1.
        const auto maxBendDepth_ = 0.5f;

        // Vertical drag distance (pixels) that moves the bend across its whole
        // -1..1 range.
        const auto bendDragSpan_ = 200.0f;

        // Vertical offset (normalised) added to a straight segment at fraction f
        // for a signed bend in -1..1. A bend of 0 leaves the segment straight;
        // the sign chooses the bow direction and the magnitude its depth. The
        // bow is a half sine, so it is symmetric about the midpoint and bends
        // the segment evenly rather than leaning to one side.
        float bendOffset(float bend, float fraction)
        {
            if (std::abs(bend) < 1.0e-4f)
                return 0.0f;

            const auto depth = std::abs(bend) * maxBendDepth_;
            const auto bump = std::sin(juce::MathConstants<float>::pi * fraction);
            return (bend > 0.0f ? 1.0f : -1.0f) * depth * bump;
        }
    }

    MotionView::MotionView()
    : draggedIndex_(-1)
    , bentIndex_(-1)
    , hoveredIndex_(-1)
    , bendDragStartY_(0.0f)
    , bendDragStartBend_(0.0f)
    {
        setOpaque(true);
        reset();
    }

    std::vector<juce::Point<float>> MotionView::getPoints() const
    {
        return points_;
    }

    std::vector<float> MotionView::getBends() const
    {
        return bends_;
    }

    void MotionView::setPoints(const std::vector<juce::Point<float>>& points)
    {
        // Anything that cannot describe a full-width curve falls back to the
        // default single segment.
        if (points.size() < 2)
        {
            reset();
            return;
        }

        auto sorted = points;
        for (auto& point : sorted)
        {
            point.x = std::clamp(point.x, 0.0f, 1.0f);
            point.y = std::clamp(point.y, 0.0f, 1.0f);
        }

        std::sort(sorted.begin(), sorted.end(), [](const juce::Point<float>& a, const juce::Point<float>& b)
        {
            return a.x < b.x;
        });

        // The endpoints are pinned to the side edges regardless of the incoming
        // x values.
        sorted.front().x = 0.0f;
        sorted.back().x = 1.0f;

        points_ = sorted;
        bends_.assign(points_.size() - 1, 0.0f);
        draggedIndex_ = -1;
        bentIndex_ = -1;

        notifyChange();
        repaint();
    }

    void MotionView::reset()
    {
        points_ = { juce::Point<float>(0.0f, 0.0f), juce::Point<float>(1.0f, 1.0f) };
        bends_ = { 0.0f };
        draggedIndex_ = -1;
        bentIndex_ = -1;

        notifyChange();
        repaint();
    }

    void MotionView::setWaveformData(const std::vector<std::vector<float>>& waveformData)
    {
        waveformRenderer_.setWaveformData(waveformData);
        repaint();
    }

    void MotionView::setPlayheadPosition(float positionRatio)
    {
        waveformRenderer_.setPlayheadPosition(positionRatio);
        repaint();
    }

    void MotionView::setPlayheadVisibility(bool visible)
    {
        waveformRenderer_.setPlayheadVisibility(visible);
        repaint();
    }

    void MotionView::setReferenceLines(const std::vector<float>& linePositions)
    {
        referenceLines_ = linePositions;
        for (auto& position : referenceLines_)
            position = std::clamp(position, 0.0f, 1.0f);

        repaint();
    }

    void MotionView::paint(juce::Graphics& g)
    {
        g.fillAll(juce::Colour(30, 30, 30));

        g.setColour(juce::Colour(60, 60, 60));
        g.drawRect(getLocalBounds(), 1);

        // The backdrop layers, all drawn into the same plot area as the curve so
        // they share its coordinate space and sit behind the nodes: first the
        // faint waveform and playhead, then the numbered horizontal reference
        // lines.
        const auto area = plotArea();
        waveformRenderer_.paintWaveform(g, area, styles::foreground.withAlpha(backdropWaveformAlpha_));
        waveformRenderer_.paintPlayhead(g, area, styles::highlight.withAlpha(backdropPlayheadAlpha_));

        g.setFont(referenceLabelHeight_);
        for (auto i = static_cast<size_t>(0); i < referenceLines_.size(); ++i)
        {
            const auto y = toPixel({ 0.0f, referenceLines_[i] }).y;

            // The number sits in the reserved strip on the left, centred on the
            // line; the line itself starts just to its right.
            const auto lineStart = area.getX() + referenceLabelWidth_ + referenceLabelGap_;

            g.setColour(referenceColour_);
            g.drawHorizontalLine(juce::roundToInt(y), lineStart, area.getRight());

            const auto label = juce::String(static_cast<int>(i) + 1);
            const auto labelArea = juce::Rectangle<float>(area.getX(), y - referenceLabelHeight_ * 0.5f, referenceLabelWidth_, referenceLabelHeight_);
            g.drawText(label, labelArea, juce::Justification::centredRight, false);
        }

        // The connecting segments, drawn one at a time so the hovered or bent
        // one can be picked out and so bent segments can follow their curve.
        for (auto i = 0; i < static_cast<int>(bends_.size()); ++i)
        {
            juce::Path path;
            buildSegmentPath(path, i);

            const auto highlighted = (i == hoveredIndex_) || (i == bentIndex_);
            g.setColour(highlighted ? styles::highlight : styles::foreground);
            g.strokePath(path, juce::PathStrokeType(lineWidth_));
        }

        // The node handles drawn on top: hollow squares whose interior matches
        // the background, with the dragged one picked out in the highlight
        // colour.
        const auto side = nodeHalfSize_ * 2.0f;
        for (auto i = static_cast<size_t>(0); i < points_.size(); ++i)
        {
            const auto pixel = toPixel(points_[i]);
            const auto square = juce::Rectangle<float>(pixel.x - nodeHalfSize_, pixel.y - nodeHalfSize_, side, side);

            g.setColour(juce::Colour(30, 30, 30));
            g.fillRect(square);

            const auto outline = (static_cast<int>(i) == draggedIndex_) ? styles::highlight : styles::foreground;
            g.setColour(outline);
            g.drawRect(square, lineWidth_);
        }
    }

    void MotionView::resized()
    {
        repaint();
    }

    void MotionView::mouseDown(const juce::MouseEvent& event)
    {
        const auto position = event.position;
        const auto index = nodeAt(position);

        // Shift-click removes an interior node, or straightens a hovered segment;
        // the pinned endpoints are left untouched.
        if (event.mods.isShiftDown())
        {
            if (index >= 0 && !isEndpoint(index))
            {
                points_.erase(points_.begin() + index);
                bends_.erase(bends_.begin() + index - 1, bends_.begin() + index + 1);
                bends_.insert(bends_.begin() + index - 1, 0.0f);
                notifyChange();
                repaint();
                return;
            }

            const auto segment = segmentAt(position);
            if (segment >= 0)
            {
                bends_[static_cast<size_t>(segment)] = 0.0f;
                notifyChange();
                repaint();
            }
            return;
        }

        // Clicking an existing node starts dragging it.
        if (index >= 0)
        {
            draggedIndex_ = index;
            repaint();
            return;
        }

        // Clicking on a segment (but not a node) starts bending it.
        const auto segment = segmentAt(position);
        if (segment >= 0)
        {
            bentIndex_ = segment;
            hoveredIndex_ = segment;
            bendDragStartY_ = position.y;
            bendDragStartBend_ = bends_[static_cast<size_t>(segment)];
            repaint();
            return;
        }

        // Clicking empty space inserts a new interior node at the click position,
        // keeping the nodes sorted by x, and starts dragging it immediately. The
        // split segment is replaced by two straight ones.
        const auto normalised = toNormalised(position);
        auto insertAt = static_cast<size_t>(1);
        while (insertAt < points_.size() && points_[insertAt].x < normalised.x)
            ++insertAt;

        // Never insert before the first or after the last endpoint.
        insertAt = std::clamp(insertAt, static_cast<size_t>(1), points_.size() - 1);

        points_.insert(points_.begin() + insertAt, normalised);
        bends_.erase(bends_.begin() + insertAt - 1);
        bends_.insert(bends_.begin() + insertAt - 1, 2, 0.0f);
        draggedIndex_ = static_cast<int>(insertAt);

        notifyChange();
        repaint();
    }

    void MotionView::mouseDrag(const juce::MouseEvent& event)
    {
        // Bending a segment: vertical drag distance becomes the signed bend.
        if (bentIndex_ >= 0)
        {
            const auto delta = (bendDragStartY_ - event.position.y) / bendDragSpan_;
            bends_[static_cast<size_t>(bentIndex_)] = std::clamp(bendDragStartBend_ + delta, -1.0f, 1.0f);
            notifyChange();
            repaint();
            return;
        }

        if (draggedIndex_ < 0)
            return;

        const auto normalised = toNormalised(event.position);

        if (isEndpoint(draggedIndex_))
        {
            // Endpoints stay pinned to their edge and only move vertically.
            points_[draggedIndex_].y = normalised.y;
        }
        else
        {
            // Interior nodes move freely but stay strictly between their
            // neighbours so the curve remains a function of x.
            const auto leftX = points_[draggedIndex_ - 1].x + minGap_;
            const auto rightX = points_[draggedIndex_ + 1].x - minGap_;
            points_[draggedIndex_].x = std::clamp(normalised.x, leftX, rightX);
            points_[draggedIndex_].y = normalised.y;
        }

        notifyChange();
        repaint();
    }

    void MotionView::mouseUp(const juce::MouseEvent&)
    {
        if (draggedIndex_ < 0 && bentIndex_ < 0)
            return;

        draggedIndex_ = -1;
        bentIndex_ = -1;
        repaint();
    }

    void MotionView::mouseMove(const juce::MouseEvent& event)
    {
        // A segment only highlights when the cursor is not over a node handle.
        const auto segment = (nodeAt(event.position) >= 0) ? -1 : segmentAt(event.position);
        if (segment == hoveredIndex_)
            return;

        hoveredIndex_ = segment;
        repaint();
    }

    void MotionView::mouseExit(const juce::MouseEvent&)
    {
        if (hoveredIndex_ < 0)
            return;

        hoveredIndex_ = -1;
        repaint();
    }

    juce::Rectangle<float> MotionView::plotArea() const
    {
        return getLocalBounds().toFloat().reduced(nodeHalfSize_ + 1.0f);
    }

    juce::Point<float> MotionView::toPixel(juce::Point<float> normalised) const
    {
        const auto area = plotArea();
        const auto x = area.getX() + normalised.x * area.getWidth();

        // y is measured upwards, so a value of 1 maps to the top of the area.
        const auto y = area.getBottom() - normalised.y * area.getHeight();
        return { x, y };
    }

    juce::Point<float> MotionView::toNormalised(juce::Point<float> pixel) const
    {
        const auto area = plotArea();
        const auto width = area.getWidth();
        const auto height = area.getHeight();

        const auto x = (width > 0.0f) ? (pixel.x - area.getX()) / width : 0.0f;
        const auto y = (height > 0.0f) ? (area.getBottom() - pixel.y) / height : 0.0f;

        return { std::clamp(x, 0.0f, 1.0f), std::clamp(y, 0.0f, 1.0f) };
    }

    juce::Point<float> MotionView::pointOnSegment(int segment, float fraction) const
    {
        const auto left = points_[static_cast<size_t>(segment)];
        const auto right = points_[static_cast<size_t>(segment) + 1];

        const auto x = left.x + fraction * (right.x - left.x);
        const auto straightY = left.y + fraction * (right.y - left.y);
        const auto y = std::clamp(straightY + bendOffset(bends_[static_cast<size_t>(segment)], fraction), 0.0f, 1.0f);

        return toPixel({ x, y });
    }

    void MotionView::buildSegmentPath(juce::Path& path, int segment) const
    {
        path.startNewSubPath(pointOnSegment(segment, 0.0f));

        // A straight segment needs only its end point; a bent one is approximated
        // by a series of short lines along the power curve.
        if (std::abs(bends_[static_cast<size_t>(segment)]) < 1.0e-4f)
        {
            path.lineTo(pointOnSegment(segment, 1.0f));
            return;
        }

        for (auto step = 1; step <= bendSampleCount_; ++step)
            path.lineTo(pointOnSegment(segment, static_cast<float>(step) / bendSampleCount_));
    }

    int MotionView::nodeAt(juce::Point<float> point) const
    {
        const auto reach = nodeHalfSize_ + nodeHitMargin_;

        // Walk from the end so the most recently added node (drawn last, on top)
        // wins when handles overlap.
        for (auto i = static_cast<int>(points_.size()) - 1; i >= 0; --i)
        {
            const auto pixel = toPixel(points_[static_cast<size_t>(i)]);
            if (std::abs(point.x - pixel.x) <= reach && std::abs(point.y - pixel.y) <= reach)
                return i;
        }

        return -1;
    }

    int MotionView::segmentAt(juce::Point<float> point) const
    {
        auto closest = -1;
        auto closestDistance = segmentHitMargin_;

        // Each segment is walked as the same short pieces it is drawn with, and
        // the nearest piece within the hit margin wins.
        for (auto i = 0; i < static_cast<int>(bends_.size()); ++i)
        {
            auto previous = pointOnSegment(i, 0.0f);
            for (auto step = 1; step <= bendSampleCount_; ++step)
            {
                const auto current = pointOnSegment(i, static_cast<float>(step) / bendSampleCount_);
                juce::Point<float> nearest;
                const auto distance = juce::Line<float>(previous, current).getDistanceFromPoint(point, nearest);
                if (distance < closestDistance)
                {
                    closestDistance = distance;
                    closest = i;
                }
                previous = current;
            }
        }

        return closest;
    }

    bool MotionView::isEndpoint(int index) const
    {
        return index == 0 || index == static_cast<int>(points_.size()) - 1;
    }

    void MotionView::notifyChange() const
    {
        if (onChange)
            onChange(points_);
    }

}
