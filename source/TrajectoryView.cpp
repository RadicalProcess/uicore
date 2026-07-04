#include "UICore/TrajectoryView.h"
#include "UICore/Style.h"

#include <algorithm>

namespace rp::uicore
{

    namespace
    {
        // Radius (in pixels) of the round anchor markers, and the extra slack
        // added around one when hit testing so it is comfortable to grab.
        const auto anchorRadius_ = 5.0f;
        const auto anchorHitMargin_ = 4.0f;

        // Radius (in pixels) of the smaller control-handle knobs, and their own
        // hit-test slack.
        const auto handleRadius_ = 4.0f;
        const auto handleHitMargin_ = 4.0f;

        // Thickness of the curve, the frame and the handle lines.
        const auto curveWidth_ = 2.0f;
        const auto lineWidth_ = 1.5f;

        // Colour of the static square-and-circle reference frame: dim enough to
        // read as a backdrop without competing with the curve.
        const auto frameColour_ = juce::Colour(90, 90, 90);

        // The background the view fills itself with.
        const auto backgroundColour_ = juce::Colour(30, 30, 30);

        // Size (in pixels) of the square clear button tucked into the top-right
        // corner, and the margin kept around it.
        const auto clearButtonSize_ = 22;
        const auto clearButtonMargin_ = 6;

        // Width (in pixels) of the "waveform" toggle sitting just left of the
        // clear button, sharing its height, with a gap between the two.
        const auto waveformButtonWidth_ = 76;
        const auto waveformButtonGap_ = 6;

        // Alpha applied to the foreground colour for the waveform drawn along the
        // curve, keeping it a translucent backdrop the curve reads over.
        const auto waveformAlpha_ = 0.5f;

        // Half-extent (as a fraction of the reference square's side) the waveform
        // reaches perpendicular to the curve for a full-scale sample.
        const auto waveformAmplitudeFraction_ = 0.12f;

        // The playhead is a short line crossing the curve at right angles:
        // playheadHalfLength_ is how far it reaches either side of the curve,
        // playheadThickness_ its stroke width, and playheadTangentStep_ the small
        // step (in pixels) used to estimate the curve tangent at the playhead.
        const auto playheadHalfLength_ = 9.0f;
        const auto playheadThickness_ = 2.5f;
        const auto playheadTangentStep_ = 2.0f;

        // Fractions along a fresh straight segment where the two joining handles
        // are placed, so a new anchor connects to the previous one with a visibly
        // straight line whose control knobs sit on that line.
        const auto outHandleFraction_ = 1.0f / 3.0f;
        const auto inHandleFraction_ = 2.0f / 3.0f;
    }

    TrajectoryView::TrajectoryView()
    : selectedIndex_(-1)
    , dragMode_(Drag::None)
    , clearButton_("clear")
    , waveformButton_("waveform")
    , playheadEnabled_(false)
    , playheadPosition_(0.0f)
    {
        setOpaque(true);

        clearButton_.setButtonText("x");
        clearButton_.onClick = [this] { clear(); };
        addAndMakeVisible(clearButton_);

        waveformButton_.setButtonText("waveform");
        waveformButton_.setClickingTogglesState(true);
        waveformButton_.onClick = [this] { repaint(); };
        addAndMakeVisible(waveformButton_);
    }

    std::vector<juce::Point<float>> TrajectoryView::getAnchors() const
    {
        std::vector<juce::Point<float>> positions;
        positions.reserve(anchors_.size());
        for (const auto& anchor : anchors_)
            positions.push_back(anchor.position);

        return positions;
    }

    float TrajectoryView::getCircleDiameter() const
    {
        return squareArea().getWidth();
    }

    void TrajectoryView::clear()
    {
        if (anchors_.empty())
            return;

        anchors_.clear();
        selectedIndex_ = -1;
        dragMode_ = Drag::None;

        notifyChange();
        repaint();
    }

    void TrajectoryView::setWaveformData(const std::vector<float>& waveformData)
    {
        // The renderer holds channels; the curve carries a single mono channel.
        waveformRenderer_.setWaveformData({ waveformData });
        repaint();
    }

    void TrajectoryView::setPlayheadEnabled(bool enabled)
    {
        playheadEnabled_ = enabled;
        repaint();
    }

    void TrajectoryView::setPlayheadPosition(float position)
    {
        playheadPosition_ = std::clamp(position, 0.0f, 1.0f);
        repaint();
    }

    void TrajectoryView::paint(juce::Graphics& g)
    {
        g.fillAll(backgroundColour_);

        // The static reference frame: a centred square with an inscribed circle.
        const auto square = squareArea();
        g.setColour(frameColour_);
        g.drawRect(square, lineWidth_);
        g.drawEllipse(square, lineWidth_);

        // The bezier curve through the anchors, built once and reused for the
        // waveform and the playhead, with the optional waveform drawn along it
        // first so the curve reads over the top.
        juce::Path path;
        const auto hasCurve = anchors_.size() >= 2;
        if (hasCurve)
        {
            buildPath(path);

            if (waveformButton_.getToggleState())
            {
                const auto amplitude = square.getWidth() * waveformAmplitudeFraction_;
                waveformRenderer_.paintWaveformAlongPath(g, path, amplitude, styles::foreground.withAlpha(waveformAlpha_));
            }

            g.setColour(styles::foreground);
            g.strokePath(path, juce::PathStrokeType(curveWidth_));
        }

        // The control handles of the selected anchor, drawn under the anchor
        // markers so no marker is hidden behind a handle knob. Only the handles
        // that border an actual segment are shown, and each is drawn as a stem
        // from the anchor to a small knob.
        if (selectedIndex_ >= 0)
        {
            const auto& anchor = anchors_[static_cast<size_t>(selectedIndex_)];
            const auto centre = toPixel(anchor.position);

            const auto drawHandle = [&g, centre](juce::Point<float> knob)
            {
                g.setColour(frameColour_);
                g.drawLine({ centre, knob }, lineWidth_);
                g.setColour(styles::highlight);
                g.fillEllipse(knob.x - handleRadius_, knob.y - handleRadius_, handleRadius_ * 2.0f, handleRadius_ * 2.0f);
            };

            if (showHandleIn(selectedIndex_))
                drawHandle(toPixel(anchor.handleIn));

            if (showHandleOut(selectedIndex_))
                drawHandle(toPixel(anchor.handleOut));
        }

        // The anchor markers on top: the selected one is a filled highlight disc,
        // the others hollow foreground rings.
        for (auto i = static_cast<size_t>(0); i < anchors_.size(); ++i)
        {
            const auto centre = toPixel(anchors_[i].position);
            const auto bounds = juce::Rectangle<float>(centre.x - anchorRadius_, centre.y - anchorRadius_, anchorRadius_ * 2.0f, anchorRadius_ * 2.0f);

            if (static_cast<int>(i) == selectedIndex_)
            {
                g.setColour(styles::highlight);
                g.fillEllipse(bounds);
            }
            else
            {
                g.setColour(backgroundColour_);
                g.fillEllipse(bounds);
                g.setColour(styles::foreground);
                g.drawEllipse(bounds, lineWidth_);
            }
        }

        // The playhead riding along the curve, drawn on top of everything so the
        // playback position stays visible. It is a short line crossing the curve
        // perpendicular to its tangent, in the highlight colour, to match the
        // Waveform and MotionView playheads.
        if (hasCurve && playheadEnabled_)
        {
            const auto totalLength = path.getLength();
            const auto distance = playheadPosition_ * totalLength;
            const auto centre = path.getPointAlongPath(distance);

            // Estimate the tangent from points either side of the playhead. A
            // tight spot in the curve can make a small step degenerate (both
            // points coincide), so widen the step until it yields a direction;
            // this keeps the crossing line perpendicular everywhere. Only a
            // zero-length curve leaves the fallback vertical normal in place.
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

    void TrajectoryView::resized()
    {
        clearButton_.setBounds(getWidth() - clearButtonSize_ - clearButtonMargin_, clearButtonMargin_, clearButtonSize_, clearButtonSize_);

        // The waveform toggle sits just left of the clear button, sharing its
        // row and height.
        const auto waveformButtonX = clearButton_.getX() - waveformButtonGap_ - waveformButtonWidth_;
        waveformButton_.setBounds(waveformButtonX, clearButtonMargin_, waveformButtonWidth_, clearButtonSize_);

        repaint();
    }

    void TrajectoryView::mouseDown(const juce::MouseEvent& event)
    {
        const auto position = event.position;

        // Grabbing a handle of the selected anchor takes priority so handles that
        // sit near their anchor are still reachable. Shift-clicking a handle
        // resets it to its straight-line default instead of dragging.
        const auto handle = handleAt(position);
        if (handle != Drag::None)
        {
            if (event.mods.isShiftDown())
            {
                auto& anchor = anchors_[static_cast<size_t>(selectedIndex_)];
                if (handle == Drag::HandleOut)
                    anchor.handleOut = defaultHandleOut(selectedIndex_);
                else
                    anchor.handleIn = defaultHandleIn(selectedIndex_);

                notifyChange();
                repaint();
                return;
            }

            dragMode_ = handle;
            return;
        }

        const auto index = anchorAt(position);
        if (index >= 0)
        {
            // Shift-click removes the anchor and clears the selection; a plain
            // click selects it and arms a possible move.
            if (event.mods.isShiftDown())
            {
                anchors_.erase(anchors_.begin() + index);
                selectedIndex_ = -1;
                dragMode_ = Drag::None;
                notifyChange();
                repaint();
                return;
            }

            selectedIndex_ = index;
            dragMode_ = Drag::Anchor;
            repaint();
            return;
        }

        // Shift-clicking empty space has nothing to remove.
        if (event.mods.isShiftDown())
            return;

        // Clicking empty space appends a new anchor to the single curve and
        // selects it. The new anchor joins the previous one with a straight
        // segment, so the previous anchor's out handle and the new anchor's in
        // handle are placed on the line between them.
        const auto normalised = toNormalised(position);
        Anchor anchor;
        anchor.position = normalised;
        anchor.handleIn = normalised;
        anchor.handleOut = normalised;

        if (!anchors_.empty())
        {
            auto& previous = anchors_.back();
            const auto delta = normalised - previous.position;
            previous.handleOut = previous.position + delta * outHandleFraction_;
            anchor.handleIn = previous.position + delta * inHandleFraction_;
        }

        anchors_.push_back(anchor);
        selectedIndex_ = static_cast<int>(anchors_.size()) - 1;
        dragMode_ = Drag::Anchor;

        notifyChange();
        repaint();
    }

    void TrajectoryView::mouseDrag(const juce::MouseEvent& event)
    {
        if (dragMode_ == Drag::None || selectedIndex_ < 0)
            return;

        auto& anchor = anchors_[static_cast<size_t>(selectedIndex_)];
        const auto normalised = toNormalised(event.position);

        switch (dragMode_)
        {
            case Drag::Anchor:
            {
                // Move the anchor and carry both handles along with it.
                const auto delta = normalised - anchor.position;
                anchor.position = normalised;
                anchor.handleIn += delta;
                anchor.handleOut += delta;
                break;
            }

            case Drag::HandleOut:
                anchor.handleOut = normalised;
                break;

            case Drag::HandleIn:
                anchor.handleIn = normalised;
                break;

            case Drag::None:
                break;
        }

        notifyChange();
        repaint();
    }

    void TrajectoryView::mouseUp(const juce::MouseEvent&)
    {
        dragMode_ = Drag::None;
    }

    juce::Rectangle<float> TrajectoryView::squareArea() const
    {
        const auto bounds = getLocalBounds().toFloat().reduced(anchorRadius_ + 1.0f);
        const auto side = std::min(bounds.getWidth(), bounds.getHeight());
        return juce::Rectangle<float>(side, side).withCentre(bounds.getCentre());
    }

    juce::Point<float> TrajectoryView::toPixel(juce::Point<float> normalised) const
    {
        // Normalised coordinates map into the reference square, which is always
        // 1:1, so the curve keeps its proportions when the component is resized.
        const auto area = squareArea();
        return { area.getX() + normalised.x * area.getWidth(), area.getY() + normalised.y * area.getHeight() };
    }

    juce::Point<float> TrajectoryView::toNormalised(juce::Point<float> pixel) const
    {
        const auto area = squareArea();
        const auto width = area.getWidth();
        const auto height = area.getHeight();

        const auto x = (width > 0.0f) ? (pixel.x - area.getX()) / width : 0.0f;
        const auto y = (height > 0.0f) ? (pixel.y - area.getY()) / height : 0.0f;

        return { std::clamp(x, 0.0f, 1.0f), std::clamp(y, 0.0f, 1.0f) };
    }

    bool TrajectoryView::showHandleIn(int index) const
    {
        // The incoming handle only shapes a segment when a previous anchor exists.
        return index > 0;
    }

    bool TrajectoryView::showHandleOut(int index) const
    {
        // The outgoing handle only shapes a segment when a next anchor exists.
        return index < static_cast<int>(anchors_.size()) - 1;
    }

    juce::Point<float> TrajectoryView::defaultHandleIn(int index) const
    {
        const auto& anchor = anchors_[static_cast<size_t>(index)];
        const auto& previous = anchors_[static_cast<size_t>(index) - 1];
        return anchor.position + (previous.position - anchor.position) * outHandleFraction_;
    }

    juce::Point<float> TrajectoryView::defaultHandleOut(int index) const
    {
        const auto& anchor = anchors_[static_cast<size_t>(index)];
        const auto& next = anchors_[static_cast<size_t>(index) + 1];
        return anchor.position + (next.position - anchor.position) * outHandleFraction_;
    }

    void TrajectoryView::buildPath(juce::Path& path) const
    {
        path.startNewSubPath(toPixel(anchors_.front().position));

        for (auto i = static_cast<size_t>(1); i < anchors_.size(); ++i)
        {
            const auto& previous = anchors_[i - 1];
            const auto& current = anchors_[i];
            path.cubicTo(toPixel(previous.handleOut), toPixel(current.handleIn), toPixel(current.position));
        }
    }

    int TrajectoryView::anchorAt(juce::Point<float> point) const
    {
        const auto reach = anchorRadius_ + anchorHitMargin_;

        // Walk from the end so the most recently added anchor, drawn last, wins
        // when markers overlap.
        for (auto i = static_cast<int>(anchors_.size()) - 1; i >= 0; --i)
        {
            const auto centre = toPixel(anchors_[static_cast<size_t>(i)].position);
            if (point.getDistanceFrom(centre) <= reach)
                return i;
        }

        return -1;
    }

    TrajectoryView::Drag TrajectoryView::handleAt(juce::Point<float> point) const
    {
        if (selectedIndex_ < 0)
            return Drag::None;

        const auto reach = handleRadius_ + handleHitMargin_;
        const auto& anchor = anchors_[static_cast<size_t>(selectedIndex_)];

        // Only handles that are actually shown can be grabbed.
        if (showHandleOut(selectedIndex_) && point.getDistanceFrom(toPixel(anchor.handleOut)) <= reach)
            return Drag::HandleOut;

        if (showHandleIn(selectedIndex_) && point.getDistanceFrom(toPixel(anchor.handleIn)) <= reach)
            return Drag::HandleIn;

        return Drag::None;
    }

    void TrajectoryView::notifyChange() const
    {
        if (onChange)
            onChange();
    }

}
