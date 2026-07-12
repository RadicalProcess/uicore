#pragma once

#include "WaveformRenderer.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <vector>

namespace rp::uicore
{

    // A single-path cubic bezier trajectory editor drawn over a static reference
    // frame. The frame is a square whose side is the smaller of the component's
    // width and height, centred in the component, with a circle inscribed in it;
    // both are re-sized whenever the component bounds change.
    //
    // The user builds one multi-segment bezier curve by clicking empty space to
    // append an anchor, and each new anchor becomes the selected one. A newly
    // appended anchor joins the previous one with a straight segment (its handles
    // start on that line). Clicking an existing anchor selects it; shift-clicking
    // an anchor removes it. Only the selected anchor shows its control handles,
    // so at most one anchor is adjustable at a time, and the two handles move
    // independently. An anchor shows a handle only for a segment it actually
    // borders, so the first anchor shows only its outgoing handle, the last only
    // its incoming one, and a lone anchor shows none. Dragging the selected
    // anchor moves it (its handles follow), and dragging one of its handles
    // reshapes the adjoining segment; shift-clicking a handle resets it to the
    // straight-line default. There is only ever a single curve. A small button
    // in the top-right corner clears it.
    //
    // Every anchor carries a small white number label above its marker, counting
    // 1, 2, 3... from the start of the curve to the end (see NodeLabel). The
    // label follows the anchor as it is dragged, and removing an anchor
    // renumbers the remaining ones from 1 so the sequence never has gaps.
    //
    // Anchor positions and handles are stored normalised to the reference square
    // (0..1 on each axis, y increasing downwards) rather than the whole
    // component, so the curve keeps its proportions inside the circle when the
    // component is resized non-uniformly.
    class TrajectoryView : public juce::Component
    {
    public:
        // Receives notifications whenever the curve or the anchor selection
        // changes, following the JUCE listener convention. Register with
        // addListener and deregister with removeListener. A companion view
        // (such as ElevationView) is kept in sync by a host that listens here
        // and pushes the anchors on, so the two views never reference each
        // other.
        class Listener
        {
        public:
            virtual ~Listener() = default;

            // Called whenever the curve changes, either through user
            // interaction or clear(). Read the new state back through the
            // given view, e.g. getAnchors().
            virtual void trajectoryChanged(TrajectoryView* view) = 0;

            // Called whenever a different anchor becomes selected, or the
            // selection is cleared. selectedIndex is the index of the newly
            // selected anchor, or -1 when nothing is selected. Does nothing by
            // default, so listeners that only track the curve need not
            // override it.
            virtual void anchorSelectionChanged(TrajectoryView* view, int selectedIndex);

            // Called once a user edit of the curve is complete: on mouse
            // release after the curve changed during that press-drag cycle,
            // and after the curve is cleared. Not called during dragging (see
            // trajectoryChanged for per-change notifications) or for clicks
            // that only change the selection. Meant for hosts that persist
            // the curve, so they can commit it once per completed edit. Does
            // nothing by default.
            virtual void trajectoryEditEnded(TrajectoryView* view);
        };

        // One point on the curve: its position plus the two control points that
        // shape the segments on either side of it. handleIn steers the segment
        // arriving from the previous anchor and handleOut the one going to the
        // next; the two move independently. All values are normalised to 0..1.
        //
        // handleInCustomised / handleOutCustomised record whether the user has
        // dragged that handle away from its straight-line default. An untouched
        // handle is kept on the line between its anchors, so a segment stays
        // straight until a bordering handle is actually grabbed. Dragging the
        // anchor re-straightens its untouched handles rather than dragging them
        // out of place, and resetting a handle (shift-click) clears the flag.
        struct Anchor
        {
            juce::Point<float> position;
            juce::Point<float> handleIn;
            juce::Point<float> handleOut;
            bool handleInCustomised = false;
            bool handleOutCustomised = false;
        };

        TrajectoryView();
        ~TrajectoryView() override = default;

        void addListener(Listener* listener);
        void removeListener(Listener* listener);

        // The anchor positions of the curve, in the order they were added,
        // normalised to 0..1.
        std::vector<juce::Point<float>> getAnchors() const;

        // The full curve data (positions, handles and customised flags) in the
        // order the anchors were added, normalised to 0..1.
        const std::vector<Anchor>& getAnchorData() const;

        // Replaces the whole curve with the given anchors, clearing the
        // selection. This is an API update rather than a user edit, so it fires
        // neither trajectoryChanged nor trajectoryEditEnded (matching
        // ElevationView::setNodes); anchorSelectionChanged still fires when a
        // selection existed before.
        void setAnchorData(const std::vector<Anchor>& anchors);

        // The diameter (in pixels) of the inscribed reference circle, i.e. the
        // side of the centred reference square. Tracks the component bounds, so
        // a companion view can match its drawing area to the circle.
        float getCircleDiameter() const;

        // Removes every anchor, leaving an empty curve.
        void clear();

        // Highlights the anchor at the given index with the same filled
        // highlight disc a selected anchor uses, or clears the highlight when
        // the index is -1 (or out of range). Meant for a host component to
        // mark the anchor that corresponds to a node being edited in a
        // companion view; it does not change the selection.
        void setHighlightedAnchor(int index);

        // Sets the audio the "waveform" toggle draws along the curve. The data
        // is a single channel of samples in -1..1; passing an empty vector
        // leaves nothing to draw. The waveform is only visible while the toggle
        // is on and the curve has at least two anchors, and is rendered with the
        // same peak-line algorithm the Waveform component uses for dense audio
        // (see WaveformRenderer::paintWaveformAlongPath).
        void setWaveformData(const std::vector<float>& waveformData);

        // Shows or hides the playhead: a short line that crosses the curve at the
        // position set by setPlayheadPosition. Hidden by default, and only ever
        // drawn while the curve has at least two anchors. The line uses the
        // highlight colour to match the Waveform and MotionView playheads.
        void setPlayheadEnabled(bool enabled);

        // The normalised position of the playhead along the curve, 0 at the start
        // point and 1 at the end point. Values are clamped to 0..1.
        void setPlayheadPosition(float position);

    private:
        void paint(juce::Graphics& g) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;

        // What the in-progress drag is manipulating.
        enum class Drag
        {
            None,
            Anchor,
            HandleOut,
            HandleIn
        };

        // The largest square centred in the component, in pixels.
        juce::Rectangle<float> squareArea() const;

        juce::Point<float> toPixel(juce::Point<float> normalised) const;
        juce::Point<float> toNormalised(juce::Point<float> pixel) const;

        // Whether the anchor at the given index borders a segment on that side,
        // and so shows the corresponding handle when selected. The incoming
        // handle is meaningful only when a previous anchor exists, the outgoing
        // one only when a next anchor exists.
        bool showHandleIn(int index) const;
        bool showHandleOut(int index) const;

        // The straight-line default position of an anchor's handle: a third of
        // the way toward the neighbouring anchor, so the bordering segment is a
        // straight line. Only valid when the corresponding neighbour exists.
        juce::Point<float> defaultHandleIn(int index) const;
        juce::Point<float> defaultHandleOut(int index) const;

        // Re-straightens every handle bordering the anchor at the given index
        // that the user has not customised: the anchor's own two handles and the
        // neighbouring handles that point back at it. Called after an anchor is
        // moved so its adjoining segments stay straight unless a handle bordering
        // them was explicitly dragged.
        void straightenUntouchedHandles(int index);

        // Builds the cubic bezier path through the anchors (in pixels).
        void buildPath(juce::Path& path) const;

        // Index of the anchor whose marker contains the given local point, or -1.
        int anchorAt(juce::Point<float> point) const;

        // Which handle of the selected anchor the given local point is on, or
        // Drag::None when it is on neither (or nothing is selected).
        Drag handleAt(juce::Point<float> point) const;

        void notifyChange();

        // Tells every registered listener the current selection, after it has
        // changed.
        void notifySelectionChanged();

        // Tells every registered listener that a user edit of the curve is
        // complete (see Listener::trajectoryEditEnded).
        void notifyEditEnded();

        // The registered listeners, notified whenever the curve or the anchor
        // selection changes.
        juce::ListenerList<Listener> listeners_;

        std::vector<Anchor> anchors_;

        // Index of the selected anchor, or -1 when none is selected.
        int selectedIndex_;

        // Index of the externally highlighted anchor (setHighlightedAnchor),
        // or -1 when none is highlighted.
        int highlightedIndex_;

        // What the current drag is manipulating, or Drag::None when idle.
        Drag dragMode_;

        // Whether the curve changed since the last mouse press, so the mouse
        // release knows whether to announce a completed edit.
        bool curveEdited_;

        // Clears the whole curve.
        juce::TextButton clearButton_;

        // Toggles the waveform drawn along the curve, sitting just left of the
        // clear button.
        juce::TextButton waveformButton_;

        // The audio drawn along the curve while the waveform toggle is on.
        WaveformRenderer waveformRenderer_;

        // Whether the playhead marker is drawn, and its normalised position
        // (0..1) along the curve.
        bool playheadEnabled_;
        float playheadPosition_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrajectoryView)
    };

}
