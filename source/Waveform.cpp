#include "UICore/Waveform.h"
#include "UICore/Style.h"

#include <algorithm>
#include <cmath>

namespace rp::uicore
{

    namespace
    {
        // Alpha applied to the highlight colour for the translucent selection
        // overlay drawn on top of the waveform.
        const auto selectionAlpha_ = 0.3f;

        // Alpha applied to the highlight colour for the translucent triangle
        // covering the attenuated part of a fade.
        const auto fadeAlpha_ = 0.25f;

        // Half-width and height (in pixels) of the triangle fade handles drawn
        // at the top corners of the selection.
        const auto fadeHandleHalfWidth_ = 6.0f;
        const auto fadeHandleHeight_ = 9.0f;

        // Horizontal slack (in pixels) added either side of a handle when hit
        // testing, so the small triangles are comfortable to grab.
        const auto fadeHandleHitMargin_ = 4.0f;

        // Horizontal slack (in pixels) either side of a selection edge within
        // which a drag resizes the region instead of starting a new one.
        const auto selectionEdgeHitMargin_ = 5.0f;

        // Line thickness of a selection edge, plain and while it is the grab
        // target under the pointer.
        const auto selectionEdgeThickness_ = 1.0f;
        const auto highlightedEdgeThickness_ = 3.0f;

        // How much the highlighted edge is brightened over the plain one.
        const auto highlightedEdgeBrightness_ = 0.4f;

        // Colours the component falls back to when a host sets none of its
        // ColourIds.
        const auto defaultBackground_ = juce::Colour(30, 30, 30);
        const auto defaultOutline_ = juce::Colour(60, 60, 60);
        const auto defaultPlaceholderText_ = juce::Colour(120, 120, 120);
    }

    Waveform::Waveform()
        : selectionEnabled_(false)
        , selectionPersistent_(false)
        , selectionStartRatio_(0.0f)
        , selectionEndRatio_(0.0f)
        , hasSelection_(false)
        , fadeEnabled_(false)
        , fadeInRatio_(0.0f)
        , fadeOutRatio_(0.0f)
        , activeFadeHandle_(FadeHandle::None)
        , hoveredHit_(SelectionHit::None)
        , dragMode_(DragMode::None)
        , moveGrabOffsetRatio_(0.0f)
        , moveWidthRatio_(0.0f)
        , pendingAnchorRatio_(0.0f)
    {
        setOpaque(true);
        setColour(backgroundColourId, defaultBackground_);
        setColour(outlineColourId, defaultOutline_);
        setColour(traceColourId, styles::foreground);
        setColour(playheadColourId, styles::highlight);
        setColour(selectionColourId, styles::highlight);
        setColour(fadeColourId, styles::highlight);
        setColour(placeholderTextColourId, defaultPlaceholderText_);
    }

    void Waveform::paint(juce::Graphics& g)
    {
        g.fillAll(findColour(backgroundColourId));

        g.setColour(findColour(outlineColourId));
        g.drawRect(getLocalBounds(), 1);

        if (!renderer_.isEmpty())
        {
            const auto bounds = getLocalBounds().toFloat();
            renderer_.paintWaveform(g, bounds, findColour(traceColourId));
            if (hasSelection_)
                paintSelection(g);
            if (fadeHandlesVisible())
                paintFades(g);
            renderer_.paintPlayhead(g, bounds, findColour(playheadColourId));
        }
        else
        {
            g.setColour(findColour(placeholderTextColourId));
            g.setFont(juce::FontOptions(14.0f));
            g.drawText("No audio file selected", getLocalBounds(), juce::Justification::centred);
        }
    }

    void Waveform::resized()
    {
        repaint();
    }

    void Waveform::setWaveformData(const std::vector<std::vector<float>>& waveformData)
    {
        renderer_.setWaveformData(waveformData);
        repaint();
    }

    void Waveform::setPlayheadPosition(float positionRatio)
    {
        renderer_.setPlayheadPosition(positionRatio);
        repaint();
    }

    void Waveform::setPlayheadVisibility(bool visibility)
    {
        renderer_.setPlayheadVisibility(visibility);
        repaint();
    }

    void Waveform::setSelectionEnabled(bool enabled)
    {
        if (selectionEnabled_ == enabled)
            return;

        selectionEnabled_ = enabled;

        // Disabling drops any cached selection so a later re-enable starts from
        // a clean slate rather than re-exposing a stale region.
        if (!selectionEnabled_)
            clearSelection();
    }

    void Waveform::setSelection(float startRatio, float endRatio)
    {
        // The programmatic setter is also gated: when selection is disabled the
        // component holds no region at all.
        if (!selectionEnabled_)
            return;

        // Deliberately unclamped. A caller showing one slice of a longer sound
        // states the selection against that slice, so an edge the slice does not
        // reach lands outside 0..1 and has to survive being stored: clamping it
        // would silently drag that edge onto the edge of the view. Painting and
        // hit testing cope with an edge that is not on screen.
        selectionStartRatio_ = std::min(startRatio, endRatio);
        selectionEndRatio_ = std::max(startRatio, endRatio);
        hasSelection_ = true;
        resetFades();

        notifySelectionChanged();
        repaint();
    }

    void Waveform::setSelectionPersistent(bool persistent)
    {
        selectionPersistent_ = persistent;
    }

    void Waveform::clearSelection()
    {
        if (!hasSelection_)
            return;

        hasSelection_ = false;
        selectionStartRatio_ = 0.0f;
        selectionEndRatio_ = 0.0f;
        activeFadeHandle_ = FadeHandle::None;
        dragMode_ = DragMode::None;
        setHoveredHit(SelectionHit::None);
        resetFades();

        notifySelectionChanged();
        repaint();
    }

    bool Waveform::hasSelection() const
    {
        return hasSelection_;
    }

    float Waveform::getSelectionStart() const
    {
        // A resize that crossed its anchor leaves the two ratios the other way
        // round, so order them here as notifySelectionChanged does.
        return std::min(selectionStartRatio_, selectionEndRatio_);
    }

    float Waveform::getSelectionEnd() const
    {
        return std::max(selectionStartRatio_, selectionEndRatio_);
    }

    void Waveform::setFadeEnabled(bool enabled)
    {
        if (fadeEnabled_ == enabled)
            return;

        fadeEnabled_ = enabled;

        // Disabling drops any cached fades and aborts an in-progress drag so a
        // later re-enable starts without stale slopes.
        if (!fadeEnabled_)
        {
            activeFadeHandle_ = FadeHandle::None;
            resetFades();
        }

        repaint();
    }

    float Waveform::getFadeIn() const
    {
        return fadeInRatio_;
    }

    float Waveform::getFadeOut() const
    {
        return fadeOutRatio_;
    }

    void Waveform::setFade(float fadeInRatio, float fadeOutRatio)
    {
        // Gated like the selection: with fades disabled the component holds none.
        if (!fadeEnabled_)
            return;

        // Clamp each into range, then cap the fade-out against whatever the
        // fade-in leaves so the two never overlap (the same limit the drag
        // handles enforce).
        const auto clampedIn = std::clamp(fadeInRatio, 0.0f, 1.0f);
        fadeInRatio_ = clampedIn;
        fadeOutRatio_ = std::clamp(fadeOutRatio, 0.0f, 1.0f - clampedIn);

        repaint();
    }

    void Waveform::mouseDown(const juce::MouseEvent& event)
    {
        // A fade handle takes priority over starting a new selection so the user
        // can adjust a fade without disturbing the region underneath it.
        if (fadeHandlesVisible())
        {
            const auto handle = fadeHandleAt(event.getPosition());
            if (handle != FadeHandle::None)
            {
                activeFadeHandle_ = handle;
                return;
            }
        }

        if (!selectionEnabled_)
            return;

        // Landing on the existing selection reshapes it instead of replacing it,
        // so the fades it carries survive: an edge resizes, the region between
        // them slides.
        const auto hit = selectionHitAt(event.getPosition());
        const auto leftRatio = std::min(selectionStartRatio_, selectionEndRatio_);
        const auto rightRatio = std::max(selectionStartRatio_, selectionEndRatio_);

        if (hit == SelectionHit::LeftEdge || hit == SelectionHit::RightEdge)
        {
            // Re-anchoring the region so that the grabbed edge becomes the
            // moving end and the opposite edge the fixed anchor lets the
            // ordinary drag path below do the work, dragging past the anchor
            // included.
            const auto grabbingLeftEdge = hit == SelectionHit::LeftEdge;

            selectionStartRatio_ = grabbingLeftEdge ? rightRatio : leftRatio;
            selectionEndRatio_ = grabbingLeftEdge ? leftRatio : rightRatio;
            dragMode_ = DragMode::Resizing;
            setHoveredHit(hit);
            return;
        }

        if (hit == SelectionHit::Body)
        {
            // Remember where inside the region it was grabbed, so the whole
            // thing travels with the pointer rather than jumping under it.
            selectionStartRatio_ = leftRatio;
            selectionEndRatio_ = rightRatio;
            moveWidthRatio_ = rightRatio - leftRatio;
            moveGrabOffsetRatio_ = ratioForX(event.x) - leftRatio;
            dragMode_ = DragMode::Moving;
            setHoveredHit(hit);
            return;
        }

        // Remember where a new selection would be anchored, but draw nothing
        // yet: a press that never moves is a click, and a click has no region
        // to report. Waiting also means the selection already on screen is not
        // thrown away until there is something to replace it with.
        pendingAnchorRatio_ = ratioForX(event.x);
        dragMode_ = DragMode::Pending;
    }

    void Waveform::mouseDrag(const juce::MouseEvent& event)
    {
        if (activeFadeHandle_ != FadeHandle::None)
        {
            const auto leftX = selectionLeftX();
            const auto rightX = selectionRightX();
            const auto width = rightX - leftX;
            if (width <= 0.0f)
                return;

            const auto pointerX = std::clamp(static_cast<float>(event.x), leftX, rightX);

            if (activeFadeHandle_ == FadeHandle::In)
            {
                // Fade-in grows from the left edge but may not reach past the
                // start of the fade-out.
                const auto maxRatio = 1.0f - fadeOutRatio_;
                fadeInRatio_ = std::clamp((pointerX - leftX) / width, 0.0f, maxRatio);
            }
            else
            {
                // Fade-out grows from the right edge but may not reach past the
                // end of the fade-in.
                const auto maxRatio = 1.0f - fadeInRatio_;
                fadeOutRatio_ = std::clamp((rightX - pointerX) / width, 0.0f, maxRatio);
            }

            notifyFadeChanged();
            repaint();
            return;
        }

        if (!selectionEnabled_)
            return;

        if (dragMode_ == DragMode::Moving)
        {
            moveSelectionTo(ratioForX(event.x));

            notifySelectionChanged();
            repaint();
            return;
        }

        // The press has moved, so it really is a new selection after all.
        if (dragMode_ == DragMode::Pending)
        {
            selectionStartRatio_ = pendingAnchorRatio_;
            hasSelection_ = true;
            dragMode_ = DragMode::Creating;
            resetFades();
        }

        // Extend the selection to the current pointer position. The drag may go
        // either side of the anchor, so order the ratios when reporting.
        selectionEndRatio_ = ratioForX(event.x);

        // A resize that crosses the anchor turns the grabbed edge into the other
        // one, so the highlight follows the moving end rather than the edge the
        // drag started on.
        if (dragMode_ == DragMode::Resizing)
            setHoveredHit(resizedEdge());

        notifySelectionChanged();
        repaint();
    }

    void Waveform::mouseUp(const juce::MouseEvent& event)
    {
        if (activeFadeHandle_ != FadeHandle::None)
        {
            activeFadeHandle_ = FadeHandle::None;
            notifyFadeChanged();
            return;
        }

        if (!selectionEnabled_)
            return;

        const auto mode = dragMode_;
        dragMode_ = DragMode::None;

        // A press that never moved is a click, not a region. It drops whatever
        // was selected, which is the long-standing way to select nothing — but
        // not where the selection is not the component's to throw away.
        if (mode == DragMode::Pending)
        {
            if (!selectionPersistent_)
                clearSelection();

            return;
        }

        if (mode == DragMode::Moving)
        {
            moveSelectionTo(ratioForX(event.x));
        }
        else
        {
            // A resize collapsed exactly onto its own anchor describes no audio
            // either. Merely pressing an edge does not land here: it leaves the
            // region it grabbed, which is as wide as it was.
            if (!selectionPersistent_ && selectionStartRatio_ == selectionEndRatio_)
            {
                clearSelection();
                return;
            }

            selectionEndRatio_ = ratioForX(event.x);
        }

        // The pointer has stopped where it stopped, so re-test what it is over
        // now that the selection has settled.
        setHoveredHit(selectionHitAt(event.getPosition()));

        notifySelectionChanged();
        repaint();
    }

    void Waveform::mouseMove(const juce::MouseEvent& event)
    {
        setHoveredHit(selectionHitAt(event.getPosition()));
    }

    void Waveform::mouseExit(const juce::MouseEvent& /*event*/)
    {
        // A drag that leaves the component keeps its highlight; the selection is
        // still being reshaped even while the pointer is outside.
        if (dragMode_ != DragMode::None)
            return;

        setHoveredHit(SelectionHit::None);
    }

    void Waveform::paintSelection(juce::Graphics& g) const
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto width = bounds.getWidth();

        // Order the ratios so the overlay is drawn correctly regardless of the
        // drag direction.
        const auto startRatio = std::min(selectionStartRatio_, selectionEndRatio_);
        const auto endRatio = std::max(selectionStartRatio_, selectionEndRatio_);

        const auto startX = bounds.getX() + startRatio * width;
        const auto endX = bounds.getX() + endRatio * width;

        const auto selectionBounds = juce::Rectangle<float>(startX, bounds.getY(), endX - startX, bounds.getHeight());

        const auto selectionColour = findColour(selectionColourId);

        g.setColour(selectionColour.withAlpha(selectionAlpha_));
        g.fillRect(selectionBounds);

        // The edge under the pointer is drawn brighter and thicker, so it is
        // visible before the drag that resizes it starts.
        const auto highlightColour = selectionColour.brighter(highlightedEdgeBrightness_);

        const auto leftHighlighted = hoveredHit_ == SelectionHit::LeftEdge;
        g.setColour(leftHighlighted ? highlightColour : selectionColour);
        g.drawLine(startX, bounds.getY(), startX, bounds.getBottom(),
                   leftHighlighted ? highlightedEdgeThickness_ : selectionEdgeThickness_);

        const auto rightHighlighted = hoveredHit_ == SelectionHit::RightEdge;
        g.setColour(rightHighlighted ? highlightColour : selectionColour);
        g.drawLine(endX, bounds.getY(), endX, bounds.getBottom(),
                   rightHighlighted ? highlightedEdgeThickness_ : selectionEdgeThickness_);
    }

    void Waveform::paintFades(juce::Graphics& g) const
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto top = bounds.getY();
        const auto bottom = bounds.getBottom();
        const auto leftX = selectionLeftX();
        const auto rightX = selectionRightX();
        const auto width = rightX - leftX;
        if (width <= 0.0f)
            return;

        const auto fadeColour = findColour(fadeColourId);

        // The slope rises from silence (bottom) at the selection edge to full
        // gain (top) where the fade ends; the triangle it cuts off is shaded to
        // emphasise the attenuated portion.
        const auto fadeInEndX = leftX + fadeInRatio_ * width;
        {
            juce::Path covered;
            covered.startNewSubPath(leftX, top);
            covered.lineTo(fadeInEndX, top);
            covered.lineTo(leftX, bottom);
            covered.closeSubPath();

            g.setColour(fadeColour.withAlpha(fadeAlpha_));
            g.fillPath(covered);

            g.setColour(fadeColour);
            g.drawLine(leftX, bottom, fadeInEndX, top, 1.5f);
        }

        const auto fadeOutStartX = rightX - fadeOutRatio_ * width;
        {
            juce::Path covered;
            covered.startNewSubPath(fadeOutStartX, top);
            covered.lineTo(rightX, top);
            covered.lineTo(rightX, bottom);
            covered.closeSubPath();

            g.setColour(fadeColour.withAlpha(fadeAlpha_));
            g.fillPath(covered);

            g.setColour(fadeColour);
            g.drawLine(fadeOutStartX, top, rightX, bottom, 1.5f);
        }

        // Draw the grab handles last so they sit on top of the slopes. Each is a
        // small triangle hanging from the top edge at the fade boundary.
        g.setColour(fadeColour);
        for (const auto handleX : {fadeInEndX, fadeOutStartX})
        {
            juce::Path handle;
            handle.startNewSubPath(handleX - fadeHandleHalfWidth_, top);
            handle.lineTo(handleX + fadeHandleHalfWidth_, top);
            handle.lineTo(handleX, top + fadeHandleHeight_);
            handle.closeSubPath();
            g.fillPath(handle);
        }
    }

    float Waveform::ratioForX(int x) const
    {
        const auto width = getLocalBounds().getWidth();
        if (width <= 0)
            return 0.0f;

        return std::clamp(static_cast<float>(x) / static_cast<float>(width), 0.0f, 1.0f);
    }

    void Waveform::notifySelectionChanged() const
    {
        if (!onSelectionChanged)
            return;

        const auto startRatio = std::min(selectionStartRatio_, selectionEndRatio_);
        const auto endRatio = std::max(selectionStartRatio_, selectionEndRatio_);
        onSelectionChanged(startRatio, endRatio);
    }

    void Waveform::notifyFadeChanged() const
    {
        if (onFadeChanged)
            onFadeChanged(fadeInRatio_, fadeOutRatio_);
    }

    float Waveform::selectionLeftX() const
    {
        const auto width = static_cast<float>(getLocalBounds().getWidth());
        const auto startRatio = std::min(selectionStartRatio_, selectionEndRatio_);
        return startRatio * width;
    }

    float Waveform::selectionRightX() const
    {
        const auto width = static_cast<float>(getLocalBounds().getWidth());
        const auto endRatio = std::max(selectionStartRatio_, selectionEndRatio_);
        return endRatio * width;
    }

    Waveform::FadeHandle Waveform::fadeHandleAt(juce::Point<int> point) const
    {
        if (!fadeHandlesVisible())
            return FadeHandle::None;

        const auto top = static_cast<float>(getLocalBounds().getY());
        const auto pointY = static_cast<float>(point.getY());
        if (pointY < top || pointY > top + fadeHandleHeight_)
            return FadeHandle::None;

        const auto leftX = selectionLeftX();
        const auto rightX = selectionRightX();
        const auto width = rightX - leftX;
        if (width <= 0.0f)
            return FadeHandle::None;

        const auto pointX = static_cast<float>(point.getX());
        const auto hitHalfWidth = fadeHandleHalfWidth_ + fadeHandleHitMargin_;

        const auto fadeInEndX = leftX + fadeInRatio_ * width;
        const auto fadeOutStartX = rightX - fadeOutRatio_ * width;

        // When both handles sit on top of each other (full selection covered, or
        // both fades at zero on a tiny selection) the in-handle wins; the
        // out-handle can still be reached once the in-handle is dragged away.
        if (std::abs(pointX - fadeInEndX) <= hitHalfWidth)
            return FadeHandle::In;
        if (std::abs(pointX - fadeOutStartX) <= hitHalfWidth)
            return FadeHandle::Out;

        return FadeHandle::None;
    }

    bool Waveform::fadeHandlesVisible() const
    {
        return fadeEnabled_ && hasSelection_;
    }

    Waveform::SelectionHit Waveform::selectionHitAt(juce::Point<int> point) const
    {
        if (!selectionEnabled_ || !hasSelection_)
            return SelectionHit::None;

        // A fade handle overlapping the same point takes the drag, so it must
        // take the highlight and the cursor with it.
        if (fadeHandleAt(point) != FadeHandle::None)
            return SelectionHit::None;

        const auto pointX = static_cast<float>(point.getX());
        const auto leftX = selectionLeftX();
        const auto rightX = selectionRightX();
        const auto leftDistance = std::abs(pointX - leftX);
        const auto rightDistance = std::abs(pointX - rightX);

        // The edges are the finer targets, so their grab areas are tested first
        // and win over the region they enclose. On a selection narrow enough for
        // both to cover the point, the nearer edge wins.
        if (std::min(leftDistance, rightDistance) <= selectionEdgeHitMargin_)
            return leftDistance <= rightDistance ? SelectionHit::LeftEdge : SelectionHit::RightEdge;

        if (pointX > leftX && pointX < rightX)
            return SelectionHit::Body;

        return SelectionHit::None;
    }

    Waveform::SelectionHit Waveform::resizedEdge() const
    {
        return selectionEndRatio_ < selectionStartRatio_ ? SelectionHit::LeftEdge : SelectionHit::RightEdge;
    }

    void Waveform::moveSelectionTo(float pointerRatio)
    {
        // Clamping the leading edge against the width the drag started with
        // stops the region at either end instead of letting it shrink as it is
        // pushed off. A region wider than what is on show has no position where
        // it could sit inside, so it is only held to overlapping it — demanding
        // more would make it jump the moment it was grabbed.
        const auto fits = moveWidthRatio_ <= 1.0f;
        const auto lowest = fits ? 0.0f : -moveWidthRatio_;
        const auto highest = fits ? 1.0f - moveWidthRatio_ : 1.0f;

        const auto leftRatio = std::clamp(pointerRatio - moveGrabOffsetRatio_, lowest, highest);

        selectionStartRatio_ = leftRatio;
        selectionEndRatio_ = leftRatio + moveWidthRatio_;
    }

    void Waveform::setHoveredHit(SelectionHit hit)
    {
        setMouseCursor(cursorFor(hit));

        if (hoveredHit_ == hit)
            return;

        hoveredHit_ = hit;
        repaint();
    }

    juce::MouseCursor Waveform::cursorFor(SelectionHit hit) const
    {
        switch (hit)
        {
            case SelectionHit::LeftEdge:
            case SelectionHit::RightEdge:
                return juce::MouseCursor::LeftRightResizeCursor;
            case SelectionHit::Body:
                return juce::MouseCursor::DraggingHandCursor;
            case SelectionHit::None:
                break;
        }

        return juce::MouseCursor::NormalCursor;
    }

    void Waveform::resetFades()
    {
        fadeInRatio_ = 0.0f;
        fadeOutRatio_ = 0.0f;
    }
}
