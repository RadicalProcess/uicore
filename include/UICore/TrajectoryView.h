#pragma once

#include "WaveformRenderer.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <vector>

namespace rp::uicore
{

    // A single-path cubic bezier trajectory editor stacked on top of an elevation
    // editor, both inside one component. The two cannot be used apart, so they
    // live together here rather than as separate views.
    //
    // Everything is expressed in the OpenGL coordinate convention: +x points to
    // the right, +y points up (the elevation), and +z points backwards (towards
    // the viewer). Each axis runs -1..1 with the origin at the centre.
    //
    // The top area is a top-down view of the x/z ground plane: a square whose
    // side is the smaller of the area's width and height, centred, with a circle
    // inscribed in it. The user builds one multi-segment bezier curve there by
    // clicking empty space to append an anchor, and each new anchor becomes the
    // selected one. A newly appended anchor joins the previous one with a
    // straight segment (its handles start on that line). Clicking an existing
    // anchor selects it; shift-clicking an anchor removes it. Only the selected
    // anchor shows its control handles, and the two handles move independently.
    // An anchor shows a handle only for a segment it actually borders, so the
    // first anchor shows only its outgoing handle, the last only its incoming
    // one, and a lone anchor shows none. Dragging the selected anchor moves it
    // (its handles follow), and dragging one of its handles reshapes the
    // adjoining segment; shift-clicking a handle resets it to the straight-line
    // default. A small button in the top-right corner clears the whole curve.
    //
    // The bottom strip is the elevation editor. It draws a straight-segment graph
    // (no curves) through one node per trajectory anchor, laid out in a drawing
    // area whose width matches the circle above so a node lines up horizontally
    // with its anchor. Nodes cannot be added or removed there; the user can only
    // drag a node vertically to change its elevation. A horizontal reference line
    // across the middle marks elevation zero (y = 0), values above it are
    // positive (up) and below it negative.
    //
    // Every anchor and its elevation node carry a small white number label above
    // the marker, counting 1, 2, 3... from the start of the curve to the end (see
    // NodeLabel); removing an anchor renumbers the rest so the sequence never has
    // gaps.
    class TrajectoryView : public juce::Component
    {
    public:
        // Receives notifications whenever the curve or the elevations change,
        // following the JUCE listener convention. Register with addListener and
        // deregister with removeListener.
        class Listener
        {
        public:
            virtual ~Listener() = default;

            // Called whenever the curve or an elevation changes, either through
            // user interaction or clear(). Read the new state back through the
            // given view, e.g. getAnchorData().
            virtual void trajectoryChanged(TrajectoryView* view) = 0;

            // Called once a user edit is complete: on mouse release after the
            // state changed during that press-drag cycle, and after the curve is
            // cleared. Not called during dragging (see trajectoryChanged for
            // per-change notifications) or for clicks that only change the
            // selection. Meant for hosts that persist the trajectory, so they can
            // commit it once per completed edit. Does nothing by default.
            virtual void trajectoryEditEnded(TrajectoryView* view);
        };

        // One node of the curve. position is the anchor on the x/z ground plane
        // (its .x is the OpenGL x, its .y is the OpenGL z), handleIn/handleOut are
        // the bezier control points on either side of it in that same plane, and
        // elevation is the node's OpenGL y (up). handleIn steers the segment
        // arriving from the previous anchor and handleOut the one going to the
        // next; the two move independently. All values are in the -1..1 OpenGL
        // convention; a handle's elevation is not modelled (the elevation between
        // nodes is a straight line, not a curve).
        //
        // handleInCustomised / handleOutCustomised record whether the user has
        // dragged that handle away from its straight-line default. An untouched
        // handle is kept on the line between its anchors, so a segment stays
        // straight until a bordering handle is actually grabbed.
        struct Anchor
        {
            juce::Point<float> position;
            juce::Point<float> handleIn;
            juce::Point<float> handleOut;
            bool handleInCustomised = false;
            bool handleOutCustomised = false;
            float elevation = 0.0f;
        };

        TrajectoryView();
        ~TrajectoryView() override = default;

        void addListener(Listener* listener);
        void removeListener(Listener* listener);

        // The full curve data (positions, handles, customised flags and
        // elevations) in the order the anchors were added.
        const std::vector<Anchor>& getAnchorData() const;

        // Replaces the whole curve with the given anchors, clearing the
        // selection. This is an API update rather than a user edit, so it fires
        // neither trajectoryChanged nor trajectoryEditEnded, so a host restoring
        // a stored curve does not immediately hear it back.
        void setAnchorData(const std::vector<Anchor>& anchors);

        // Removes every anchor, leaving an empty curve.
        void clear();

        // Sets the audio the "waveform" toggle draws along the curve. The data is
        // a single channel of samples in -1..1; passing an empty vector leaves
        // nothing to draw. The waveform is only visible while the toggle is on and
        // the curve has at least two anchors.
        void setWaveformData(const std::vector<float>& waveformData);

        // Allows or blocks user edits. Editable by default. While not editable,
        // every mouse interaction on the canvas is ignored and the clear button
        // is disabled; disabling also deselects the current anchor (hiding its
        // handles and highlight) and aborts any in-progress drag, committing the
        // changes that drag already made (see trajectoryEditEnded). The playhead
        // and the waveform toggle stay live, so a host can show pure playback.
        void setEditable(bool editable);

        // Shows or hides the playhead: short lines crossing the curve and the
        // elevation graph at the position set by setPlayheadPosition. Hidden by
        // default, and only ever drawn while there are at least two anchors.
        void setPlayheadEnabled(bool enabled);

        // The normalised position of the playhead along the curve, 0 at the start
        // and 1 at the end. Values are clamped to 0..1.
        void setPlayheadPosition(float position);

        // Builds the cubic bezier path through the given anchors, mapping their
        // -1..1 ground-plane coordinates into the given square (in pixels). Static
        // so a companion view such as TrajectoryThumbnail can render the exact
        // same curve at its own scale.
        static void buildPath(const std::vector<Anchor>& anchors,
                              const juce::Rectangle<float>& square,
                              juce::Path& path);

    private:
        void paint(juce::Graphics& g) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;

        // What the in-progress plane drag is manipulating.
        enum class Drag
        {
            None,
            Anchor,
            HandleOut,
            HandleIn
        };

        // The area the ground-plane editor draws in (the whole component minus
        // the elevation strip at the bottom).
        juce::Rectangle<float> planeArea() const;

        // The largest square centred in the plane area, in pixels.
        juce::Rectangle<float> squareArea() const;

        // The drawing area of the elevation strip: as wide as the square above and
        // centred under it, in the bottom band of the component.
        juce::Rectangle<float> elevationArea() const;

        juce::Point<float> toPixel(juce::Point<float> normalised) const;
        juce::Point<float> toNormalised(juce::Point<float> pixel) const;

        // The pixel position of the elevation node at the given index, from its
        // anchor's x and its elevation.
        juce::Point<float> elevationNodePixel(int index) const;

        // Index of the elevation node whose marker contains the given local point,
        // or -1.
        int elevationNodeAt(juce::Point<float> point) const;

        // Whether the anchor at the given index borders a segment on that side,
        // and so shows the corresponding handle when selected.
        bool showHandleIn(int index) const;
        bool showHandleOut(int index) const;

        // The straight-line default position of an anchor's handle: a third of the
        // way toward the neighbouring anchor. Only valid when the corresponding
        // neighbour exists.
        juce::Point<float> defaultHandleIn(int index) const;
        juce::Point<float> defaultHandleOut(int index) const;

        // Re-straightens every handle bordering the anchor at the given index that
        // the user has not customised, called after an anchor is moved so its
        // adjoining segments stay straight unless a bordering handle was dragged.
        void straightenUntouchedHandles(int index);

        // Index of the anchor whose marker contains the given local point, or -1.
        int anchorAt(juce::Point<float> point) const;

        // Which handle of the selected anchor the given local point is on, or
        // Drag::None when it is on neither (or nothing is selected).
        Drag handleAt(juce::Point<float> point) const;

        void paintPlane(juce::Graphics& g);
        void paintElevation(juce::Graphics& g);

        void notifyChange();
        void notifyEditEnded();

        juce::ListenerList<Listener> listeners_;

        std::vector<Anchor> anchors_;

        // Index of the selected anchor, or -1 when none is selected.
        int selectedIndex_;

        // What the current plane drag is manipulating, or Drag::None when idle.
        Drag dragMode_;

        // Index of the elevation node being dragged, or -1 when idle.
        int dragElevationIndex_;

        // Whether the trajectory changed since the last mouse press, so the mouse
        // release knows whether to announce a completed edit.
        bool curveEdited_;

        // Whether user edits are accepted (see setEditable).
        bool editable_;

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
