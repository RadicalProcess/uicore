#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <vector>

namespace rp::uicore
{

    // A companion editor for TrajectoryView that sets the elevation of each
    // trajectory node. It draws a straight-segment graph (no curves) through a
    // row of nodes: the nodes are spread evenly across a drawing area whose width
    // is fixed by setDrawingAreaWidth, and the graph is centred in the component.
    //
    // The set of nodes is driven entirely by the API: setNodes replaces them and
    // the user cannot add or remove nodes by clicking. The user can only drag a
    // node vertically to change its elevation; a node never moves horizontally.
    // Each drag fires onChange so the host can read the new elevations back.
    //
    // A horizontal reference line across the middle of the drawing area marks
    // elevation zero. Node values are normalised to the drawing area (0 at the
    // top, 1 at the bottom), so 0.5 is elevation zero and values above 0.5 sit
    // below the line, letting the user specify negative elevation.
    class ElevationView : public juce::Component
    {
    public:
        ElevationView();
        ~ElevationView() override = default;

        // Replaces the nodes with the given elevations, one per node, in order.
        // Each value is a normalised vertical position in the drawing area
        // (0 = top, 1 = bottom, 0.5 = elevation zero) and is clamped to 0..1.
        // The nodes are spread evenly across the drawing area width. This does
        // not fire onChange, since it is an API update rather than a user edit.
        void setNodes(const std::vector<float>& positions);

        // The current node elevations, normalised 0..1, in node order.
        std::vector<float> getNodes() const;

        // Sets the width (in pixels) of the drawing area the graph is laid out
        // in. The area stays centred in the component; a width wider than the
        // component is clamped to what fits.
        void setDrawingAreaWidth(float width);

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
        // even horizontal slot and its normalised elevation.
        juce::Point<float> nodePixel(int index) const;

        // Index of the node whose marker contains the given local point, or -1.
        int nodeAt(juce::Point<float> point) const;

        void notifyChange() const;

        // Normalised elevations, one per node (0 = top, 1 = bottom).
        std::vector<float> values_;

        // Width (in pixels) of the drawing area, or 0 before it is set (in which
        // case the whole component width is used).
        float drawingAreaWidth_;

        // Index of the node being dragged, or -1 when idle.
        int dragIndex_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ElevationView)
    };

}
