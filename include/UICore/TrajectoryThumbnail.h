#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <vector>

namespace rp::uicore
{
    // A miniature, non-interactive preview of a gesture's trajectory: the same
    // cubic bezier curve the TrajectoryView editor draws, scaled into this
    // component's own inscribed square. Only the curve is drawn - no anchors,
    // handles or reference frame - so it stays readable at row-thumbnail size.
    // Feed it the curve with setAnchorData; fewer than two anchors leave a
    // plain background tile.
    //
    // The TrajectoryView editor itself moved to the SGD layer; Anchor and
    // buildPath are hosted here until this thumbnail follows it there.
    class TrajectoryThumbnail : public juce::Component
    {
    public:
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

        TrajectoryThumbnail();

        void setAnchorData(const std::vector<Anchor>& anchors);

        void paint(juce::Graphics &) override;

        // Builds the cubic bezier path through the given anchors, mapping their
        // -1..1 ground-plane coordinates into the given square (in pixels). Static
        // so a companion view such as the TrajectoryView editor can render the
        // exact same curve at its own scale.
        static void buildPath(const std::vector<Anchor>& anchors,
                              const juce::Rectangle<float>& square,
                              juce::Path& path);

    private:
        std::vector<Anchor> anchors_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrajectoryThumbnail)
    };

}
