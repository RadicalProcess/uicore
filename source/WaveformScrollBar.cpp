#include "UICore/WaveformScrollBar.h"
#include "UICore/Style.h"

#include <algorithm>
#include <cmath>

namespace rp::uicore
{

    namespace
    {
        // Alpha applied to the thumb colour for the translucent body of the
        // thumb.
        const auto thumbAlpha_ = 0.22f;

        // Line thickness of a thumb edge, plain and while it is the grab target
        // under the pointer, and how much brighter the grab target is drawn.
        const auto thumbEdgeThickness_ = 1.5f;
        const auto hoveredEdgeThickness_ = 3.0f;
        const auto hoveredEdgeBrightness_ = 0.4f;

        // Horizontal slack (in pixels) either side of a thumb edge within which
        // a drag resizes the view instead of sliding it.
        const auto thumbEdgeHitMargin_ = 5.0f;

        // The marked range spans the full height, so it reads as a region of the
        // sound rather than a stripe under it. Its alpha is low enough that the
        // thumb still tells over the top of it.
        const auto markerAlphaOverFullHeight_ = 0.3f;

        // The narrowest the view may be dragged, as a ratio of the whole sound,
        // so the thumb never becomes too small to grab.
        const auto minimumViewWidth_ = 0.002f;

        // Colours the component falls back to when a host sets none of its
        // ColourIds.
        const auto defaultBackground_ = juce::Colour(20, 20, 20);
        const auto defaultOutline_ = juce::Colour(60, 60, 60);
    }

    WaveformScrollBar::WaveformScrollBar()
        : viewStartRatio_(0.0f)
        , viewEndRatio_(1.0f)
        , markedStartRatio_(0.0f)
        , markedEndRatio_(0.0f)
        , hoveredHit_(ThumbHit::None)
        , draggedHit_(ThumbHit::None)
        , grabOffsetRatio_(0.0f)
        , grabWidthRatio_(0.0f)
    {
        setOpaque(true);
        setColour(backgroundColourId, defaultBackground_);
        setColour(outlineColourId, defaultOutline_);
        setColour(traceColourId, styles::foreground);
        setColour(thumbColourId, styles::highlight);
        setColour(markerColourId, styles::foreground);
    }

    void WaveformScrollBar::paint(juce::Graphics& g)
    {
        const auto bounds = getLocalBounds().toFloat();

        g.fillAll(findColour(backgroundColourId));

        if (!renderer_.isEmpty())
            renderer_.paintWaveform(g, bounds, findColour(traceColourId));

        // The marked range first and underneath: it is reference, not the
        // control.
        if (markedEndRatio_ > markedStartRatio_)
        {
            const auto markerLeft = bounds.getX() + markedStartRatio_ * bounds.getWidth();
            const auto markerRight = bounds.getX() + markedEndRatio_ * bounds.getWidth();

            g.setColour(findColour(markerColourId).withAlpha(markerAlphaOverFullHeight_));
            g.fillRect(juce::Rectangle<float>(markerLeft, bounds.getY(), markerRight - markerLeft, bounds.getHeight()));
        }

        const auto leftX = thumbLeftX();
        const auto rightX = thumbRightX();
        const auto thumbColour = findColour(thumbColourId);
        const auto hoveredColour = thumbColour.brighter(hoveredEdgeBrightness_);

        g.setColour(thumbColour.withAlpha(thumbAlpha_));
        g.fillRect(juce::Rectangle<float>(leftX, bounds.getY(), rightX - leftX, bounds.getHeight()));

        const auto leftHovered = hoveredHit_ == ThumbHit::LeftEdge;
        g.setColour(leftHovered ? hoveredColour : thumbColour);
        g.drawLine(leftX, bounds.getY(), leftX, bounds.getBottom(),
                   leftHovered ? hoveredEdgeThickness_ : thumbEdgeThickness_);

        const auto rightHovered = hoveredHit_ == ThumbHit::RightEdge;
        g.setColour(rightHovered ? hoveredColour : thumbColour);
        g.drawLine(rightX, bounds.getY(), rightX, bounds.getBottom(),
                   rightHovered ? hoveredEdgeThickness_ : thumbEdgeThickness_);

        g.setColour(findColour(outlineColourId));
        g.drawRect(getLocalBounds(), 1);
    }

    void WaveformScrollBar::setWaveformData(const std::vector<std::vector<float>>& waveformData)
    {
        renderer_.setWaveformData(waveformData);
        repaint();
    }

    void WaveformScrollBar::setView(float startRatio, float endRatio)
    {
        const auto clampedStart = std::clamp(std::min(startRatio, endRatio), 0.0f, 1.0f);
        const auto clampedEnd = std::clamp(std::max(startRatio, endRatio), 0.0f, 1.0f);

        placeView(clampedStart, std::max(clampedEnd - clampedStart, minimumViewWidth_));

        repaint();
    }

    float WaveformScrollBar::getViewStart() const
    {
        return viewStartRatio_;
    }

    float WaveformScrollBar::getViewEnd() const
    {
        return viewEndRatio_;
    }

    void WaveformScrollBar::setMarkedRange(float startRatio, float endRatio)
    {
        markedStartRatio_ = std::clamp(std::min(startRatio, endRatio), 0.0f, 1.0f);
        markedEndRatio_ = std::clamp(std::max(startRatio, endRatio), 0.0f, 1.0f);

        repaint();
    }

    void WaveformScrollBar::mouseDown(const juce::MouseEvent& event)
    {
        const auto hit = thumbHitAt(event.getPosition());

        grabWidthRatio_ = viewEndRatio_ - viewStartRatio_;

        if (hit == ThumbHit::None)
        {
            // A click on the track jumps the thumb to it, centred, and then
            // behaves as though the body had been grabbed in the middle.
            draggedHit_ = ThumbHit::Body;
            grabOffsetRatio_ = grabWidthRatio_ * 0.5f;
            dragTo(ratioForX(event.x));
            setHoveredHit(ThumbHit::Body);
            return;
        }

        draggedHit_ = hit;
        grabOffsetRatio_ = ratioForX(event.x) - viewStartRatio_;
        setHoveredHit(hit);
    }

    void WaveformScrollBar::mouseDrag(const juce::MouseEvent& event)
    {
        if (draggedHit_ == ThumbHit::None)
            return;

        dragTo(ratioForX(event.x));
    }

    void WaveformScrollBar::mouseUp(const juce::MouseEvent& event)
    {
        if (draggedHit_ == ThumbHit::None)
            return;

        draggedHit_ = ThumbHit::None;
        setHoveredHit(thumbHitAt(event.getPosition()));
    }

    void WaveformScrollBar::mouseMove(const juce::MouseEvent& event)
    {
        setHoveredHit(thumbHitAt(event.getPosition()));
    }

    void WaveformScrollBar::mouseExit(const juce::MouseEvent& /*event*/)
    {
        // A drag that leaves the component keeps its highlight; the thumb is
        // still being moved even while the pointer is outside.
        if (draggedHit_ != ThumbHit::None)
            return;

        setHoveredHit(ThumbHit::None);
    }

    void WaveformScrollBar::dragTo(float pointerRatio)
    {
        if (draggedHit_ == ThumbHit::LeftEdge)
        {
            // The far edge stays put and the grabbed one follows, so the drag
            // changes how much is on show rather than where.
            const auto start = std::min(pointerRatio, viewEndRatio_ - minimumViewWidth_);
            placeView(start, viewEndRatio_ - start);
        }
        else if (draggedHit_ == ThumbHit::RightEdge)
        {
            const auto end = std::max(pointerRatio, viewStartRatio_ + minimumViewWidth_);
            placeView(viewStartRatio_, end - viewStartRatio_);
        }
        else
        {
            placeView(pointerRatio - grabOffsetRatio_, grabWidthRatio_);
        }

        notifyViewChanged();
        repaint();
    }

    void WaveformScrollBar::placeView(float start, float width)
    {
        const auto clampedWidth = std::clamp(width, minimumViewWidth_, 1.0f);

        viewStartRatio_ = std::clamp(start, 0.0f, 1.0f - clampedWidth);
        viewEndRatio_ = viewStartRatio_ + clampedWidth;
    }

    float WaveformScrollBar::ratioForX(int x) const
    {
        const auto width = getLocalBounds().getWidth();
        if (width <= 0)
            return 0.0f;

        return std::clamp(static_cast<float>(x) / static_cast<float>(width), 0.0f, 1.0f);
    }

    float WaveformScrollBar::thumbLeftX() const
    {
        return viewStartRatio_ * static_cast<float>(getLocalBounds().getWidth());
    }

    float WaveformScrollBar::thumbRightX() const
    {
        return viewEndRatio_ * static_cast<float>(getLocalBounds().getWidth());
    }

    WaveformScrollBar::ThumbHit WaveformScrollBar::thumbHitAt(juce::Point<int> point) const
    {
        const auto pointX = static_cast<float>(point.getX());
        const auto leftX = thumbLeftX();
        const auto rightX = thumbRightX();
        const auto leftDistance = std::abs(pointX - leftX);
        const auto rightDistance = std::abs(pointX - rightX);

        // The edges are the finer targets, so their grab areas are tested first
        // and win over the body they enclose.
        if (std::min(leftDistance, rightDistance) <= thumbEdgeHitMargin_)
            return leftDistance <= rightDistance ? ThumbHit::LeftEdge : ThumbHit::RightEdge;

        if (pointX > leftX && pointX < rightX)
            return ThumbHit::Body;

        return ThumbHit::None;
    }

    void WaveformScrollBar::setHoveredHit(ThumbHit hit)
    {
        setMouseCursor(cursorFor(hit));

        if (hoveredHit_ == hit)
            return;

        hoveredHit_ = hit;
        repaint();
    }

    juce::MouseCursor WaveformScrollBar::cursorFor(ThumbHit hit) const
    {
        switch (hit)
        {
            case ThumbHit::LeftEdge:
            case ThumbHit::RightEdge:
                return juce::MouseCursor::LeftRightResizeCursor;
            case ThumbHit::Body:
                return juce::MouseCursor::DraggingHandCursor;
            case ThumbHit::None:
                break;
        }

        return juce::MouseCursor::NormalCursor;
    }

    void WaveformScrollBar::notifyViewChanged() const
    {
        if (onViewChanged)
            onViewChanged(viewStartRatio_, viewEndRatio_);
    }
}
