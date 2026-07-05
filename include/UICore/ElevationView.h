#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <vector>

namespace rp::uicore
{

    // A companion editor for TrajectoryView that sets the elevation of each
    // trajectory node. It draws a straight-segment graph (no curves) through the
    // nodes inside a drawing area whose width is fixed by setDrawingAreaWidth,
    // and the graph is centred in the component.
    //
    // The set of nodes is driven entirely by the API: setNodes replaces them and
    // the user cannot add or remove nodes by clicking. Each node carries a fixed
    // horizontal position (typically the x of the matching trajectory anchor, so
    // the two views line up) and a vertical elevation. The user can only drag a
    // node vertically to change its elevation; a node never moves horizontally.
    // Each drag fires onChange so the host can read the new elevations back.
    //
    // A horizontal reference line across the middle of the drawing area marks
    // elevation zero. Node values are normalised to the drawing area (0 at the
    // top, 1 at the bottom), so 0.5 is elevation zero and values above 0.5 sit
    // below the line, letting the user specify negative elevation.
    //
    // Every node carries a small white number label above its marker, counting
    // 1, 2, 3... in node order (see NodeLabel). Since the host feeds the nodes
    // in trajectory-anchor order, each number matches the label of the
    // corresponding anchor in TrajectoryView.
    class ElevationView : public juce::Component
    {
    public:
        // Receives callbacks when the user grabs or releases a node. Register
        // with addListener; a host component can use this to mirror the drag
        // onto a companion view without the two views knowing each other.
        class Listener
        {
        public:
            virtual ~Listener() = default;

            // Called when the user presses on the node at the given index,
            // starting a drag.
            virtual void nodeDragStarted(ElevationView* view, int index) = 0;

            // Called when the user releases the node at the given index,
            // ending the drag.
            virtual void nodeDragEnded(ElevationView* view, int index) = 0;
        };

        ElevationView();
        ~ElevationView() override = default;

        void addListener(Listener* listener);
        void removeListener(Listener* listener);

        // Replaces the nodes with the given positions, one per node, in order.
        // Each point is normalised to the drawing area and clamped to 0..1 on
        // both axes: x is the node's fixed horizontal position (0 = left edge,
        // 1 = right edge), matching TrajectoryView's normalised anchor x so the
        // two views stay aligned, and y its elevation (0 = top, 1 = bottom,
        // 0.5 = elevation zero). This does not fire onChange, since it is an
        // API update rather than a user edit.
        void setNodes(const std::vector<juce::Point<float>>& positions);

        // The current nodes, normalised 0..1, in node order: x the fixed
        // horizontal position, y the elevation.
        std::vector<juce::Point<float>> getNodes() const;

        // Highlights the node at the given index with the same filled
        // highlight disc a dragged node uses, or clears the highlight when the
        // index is -1 (or out of range). Meant for a host component to mark
        // the node that corresponds to the anchor selected in a companion
        // view; it does not start a drag.
        void setHighlightedNode(int index);

        // Sets the width (in pixels) of the drawing area the graph is laid out
        // in. The area stays centred in the component; a width wider than the
        // component is clamped to what fits.
        void setDrawingAreaWidth(float width);

        // Shows or hides the playhead: a short line that crosses the graph at
        // the position set by setPlayheadPosition. Hidden by default, and only
        // ever drawn while the graph has at least two nodes. The line uses the
        // highlight colour to match the TrajectoryView playhead.
        void setPlayheadEnabled(bool enabled);

        // The normalised position of the playhead along the graph, 0 at the
        // first node and 1 at the last node. Values are clamped to 0..1.
        void setPlayheadPosition(float position);

        // Invoked whenever the user drags a node to a new elevation.
        std::function<void()> onChange;

    private:
        void paint(juce::Graphics& g) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;

        // The drawing area: a rectangle of the configured width, centred in the
        // component, inset vertically so node markers stay inside the bounds.
        juce::Rectangle<float> drawingArea() const;

        // The pixel position of the node at the given index, derived from its
        // fixed horizontal position and its elevation.
        juce::Point<float> nodePixel(int index) const;

        // Index of the node whose marker contains the given local point, or -1.
        int nodeAt(juce::Point<float> point) const;

        void notifyChange() const;

        juce::ListenerList<Listener> listeners_;

        // Normalised nodes: x the fixed horizontal position, y the elevation
        // (0 = top, 1 = bottom).
        std::vector<juce::Point<float>> nodes_;

        // Width (in pixels) of the drawing area, or 0 before it is set (in which
        // case the whole component width is used).
        float drawingAreaWidth_;

        // Index of the node being dragged, or -1 when idle.
        int dragIndex_;

        // Index of the externally highlighted node (setHighlightedNode), or
        // -1 when none is highlighted.
        int highlightedIndex_;

        // Whether the playhead marker is drawn, and its normalised position
        // (0..1) along the graph.
        bool playheadEnabled_;
        float playheadPosition_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ElevationView)
    };

}
