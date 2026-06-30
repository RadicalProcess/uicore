#include "UICore/MotionView.h"
#include "UICore/Style.h"

#include <algorithm>

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
    }

    MotionView::MotionView()
    : draggedIndex_(-1)
    {
        setOpaque(true);
        reset();
    }

    std::vector<juce::Point<float>> MotionView::getPoints() const
    {
        return points_;
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
        draggedIndex_ = -1;

        notifyChange();
        repaint();
    }

    void MotionView::reset()
    {
        points_ = { juce::Point<float>(0.0f, 0.0f), juce::Point<float>(1.0f, 1.0f) };
        draggedIndex_ = -1;

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

    void MotionView::paint(juce::Graphics& g)
    {
        g.fillAll(juce::Colour(30, 30, 30));

        g.setColour(juce::Colour(60, 60, 60));
        g.drawRect(getLocalBounds(), 1);

        // The waveform backdrop and playhead are drawn first, into the same plot
        // area as the curve, so they sit behind the nodes and share their
        // coordinate space.
        const auto area = plotArea();
        waveformRenderer_.paintWaveform(g, area, styles::foreground.withAlpha(backdropWaveformAlpha_));
        waveformRenderer_.paintPlayhead(g, area, styles::highlight.withAlpha(backdropPlayheadAlpha_));

        // The connecting segments.
        juce::Path path;
        for (auto i = static_cast<size_t>(0); i < points_.size(); ++i)
        {
            const auto pixel = toPixel(points_[i]);
            if (i == 0)
                path.startNewSubPath(pixel);
            else
                path.lineTo(pixel);
        }

        g.setColour(styles::foreground);
        g.strokePath(path, juce::PathStrokeType(lineWidth_));

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

        // Shift-click removes an interior node; the pinned endpoints are left
        // untouched.
        if (event.mods.isShiftDown())
        {
            if (index >= 0 && !isEndpoint(index))
            {
                points_.erase(points_.begin() + index);
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

        // Clicking empty space inserts a new interior node at the click position,
        // keeping the nodes sorted by x, and starts dragging it immediately.
        const auto normalised = toNormalised(position);
        auto insertAt = static_cast<size_t>(1);
        while (insertAt < points_.size() && points_[insertAt].x < normalised.x)
            ++insertAt;

        // Never insert before the first or after the last endpoint.
        insertAt = std::clamp(insertAt, static_cast<size_t>(1), points_.size() - 1);

        points_.insert(points_.begin() + insertAt, normalised);
        draggedIndex_ = static_cast<int>(insertAt);

        notifyChange();
        repaint();
    }

    void MotionView::mouseDrag(const juce::MouseEvent& event)
    {
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
        if (draggedIndex_ < 0)
            return;

        draggedIndex_ = -1;
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
