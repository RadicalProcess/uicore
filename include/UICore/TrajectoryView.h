#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
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
    // reshapes the adjoining segment. There is only ever a single curve. A small
    // button in the top-right corner clears it.
    //
    // Anchor positions and handles are stored normalised to the component bounds
    // (0..1 on each axis, y increasing downwards) so the curve tracks resizes.
    class TrajectoryView : public juce::Component
    {
    public:
        TrajectoryView();
        ~TrajectoryView() override = default;

        // The anchor positions of the curve, in the order they were added,
        // normalised to 0..1.
        std::vector<juce::Point<float>> getAnchors() const;

        // Removes every anchor, leaving an empty curve.
        void clear();

        // Invoked whenever the curve changes, either through user interaction or
        // clear().
        std::function<void()> onChange;

    private:
        void paint(juce::Graphics& g) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;

        // One point on the curve: its position plus the two control points that
        // shape the segments on either side of it. handleIn steers the segment
        // arriving from the previous anchor and handleOut the one going to the
        // next; the two move independently. All values are normalised to 0..1.
        struct Anchor
        {
            juce::Point<float> position;
            juce::Point<float> handleIn;
            juce::Point<float> handleOut;
        };

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

        // Builds the cubic bezier path through the anchors (in pixels).
        void buildPath(juce::Path& path) const;

        // Index of the anchor whose marker contains the given local point, or -1.
        int anchorAt(juce::Point<float> point) const;

        // Which handle of the selected anchor the given local point is on, or
        // Drag::None when it is on neither (or nothing is selected).
        Drag handleAt(juce::Point<float> point) const;

        void notifyChange() const;

        std::vector<Anchor> anchors_;

        // Index of the selected anchor, or -1 when none is selected.
        int selectedIndex_;

        // What the current drag is manipulating, or Drag::None when idle.
        Drag dragMode_;

        // Clears the whole curve.
        juce::TextButton clearButton_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrajectoryView)
    };

}
