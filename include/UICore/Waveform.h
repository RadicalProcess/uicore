#pragma once

#include "WaveformRenderer.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <vector>

namespace rp::uicore
{

    // Displays a (potentially very long) sample. In addition to a playhead, the
    // user can select a region by clicking and dragging; the selected region is
    // highlighted with a translucent overlay and reported back as a pair of
    // normalised ratios (0..1) so a caller can show that slice in another
    // editor view. An existing selection can be reshaped without being redrawn:
    // dragging either of its edges resizes it, dragging the region between them
    // slides it along at a fixed width, and a drag starting outside it replaces
    // it. The pointer says which is which, highlighting an edge it is over and
    // swapping in a resize or a dragging-hand cursor. Selection is opt-in via
    // setSelectionEnabled. The selection
    // can additionally carry a fade-in and fade-out, edited via draggable
    // triangle handles and opt-in through setFadeEnabled. Every colour it
    // paints with comes from its ColourIds so a host can restyle a single
    // waveform or all of them.
    class Waveform : public juce::Component
    {
    public:
        enum ColourIds
        {
            backgroundColourId = 0x2005000,
            outlineColourId = 0x2005001,
            traceColourId = 0x2005002,
            playheadColourId = 0x2005003,
            selectionColourId = 0x2005004,
            fadeColourId = 0x2005005,
            placeholderTextColourId = 0x2005006
        };

        Waveform();
        ~Waveform() override = default;

        void setWaveformData(const std::vector<std::vector<float>>& waveformData);

        void setPlayheadPosition(float positionRatio);

        void setPlayheadVisibility(bool visible);

        // Enables click-and-drag region selection. Disabled by default; when
        // disabled the component ignores mouse interaction and any existing
        // selection is cleared.
        void setSelectionEnabled(bool enabled);

        // Programmatically set the selection. Both arguments are ratios of what
        // the component is showing, ordered so that start <= end but NOT
        // clamped: a caller drawing one slice of a longer sound states an edge
        // beyond that slice outside 0..1, and the selection is then painted
        // running off that side. Has no effect while selection is disabled (see
        // setSelectionEnabled).
        void setSelection(float startRatio, float endRatio);

        // Makes the selection permanent: a click without a drag leaves it alone
        // instead of clearing it. Off by default. Turn it on where the selection
        // means something the component cannot invent — a range owned elsewhere,
        // which a stray click must not be able to destroy.
        void setSelectionPersistent(bool persistent);

        void clearSelection();

        bool hasSelection() const;

        // The selection edges as ratios, ordered so that start <= end however
        // the user arrived at them.
        float getSelectionStart() const;

        float getSelectionEnd() const;

        // Enables fade editing on the current selection. Disabled by default;
        // while disabled the component behaves exactly as it does without fades.
        // When enabled, two small triangle handles appear at the top corners of
        // the selection: the left one controls the fade-in and the right one the
        // fade-out. Dragging a handle inwards lengthens the corresponding fade,
        // which is drawn as a slope over the selected region.
        void setFadeEnabled(bool enabled);

        // Fade lengths expressed as ratios (0..1) of the selection width. The
        // fade-in ramps up over the first fadeInRatio of the selection and the
        // fade-out ramps down over the last fadeOutRatio. Their sum never
        // exceeds 1 (the two fades cannot overlap).
        float getFadeIn() const;

        float getFadeOut() const;

        // Programmatically set the fades. Both are ratios (0..1) of the selection
        // width; they are clamped and the fade-out is capped so the two never
        // overlap. Has no effect while fades are disabled (see setFadeEnabled).
        void setFade(float fadeInRatio, float fadeOutRatio);

        // Invoked whenever the selection changes, either through user
        // interaction or setSelection / clearSelection.
        std::function<void(float startRatio, float endRatio)> onSelectionChanged;

        // Invoked whenever a fade changes through user interaction, both during
        // the drag and once the mouse is released. Ratios are relative to the
        // selection width (see getFadeIn / getFadeOut).
        std::function<void(float fadeInRatio, float fadeOutRatio)> onFadeChanged;

    private:
        // Identifies which fade handle, if any, a point is over or is being
        // dragged.
        enum class FadeHandle
        {
            None,
            In,
            Out
        };

        // Identifies what part of the selection, if any, a point is over: one of
        // its edges, which a drag resizes, or the region between them, which a
        // drag slides.
        enum class SelectionHit
        {
            None,
            LeftEdge,
            RightEdge,
            Body
        };

        // What the drag in progress is doing to the selection. Pending is a
        // press that has not moved yet: it may still become a new selection, so
        // nothing is disturbed until it does.
        enum class DragMode
        {
            None,
            Pending,
            Creating,
            Resizing,
            Moving
        };

        void paint(juce::Graphics& g) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;
        void mouseMove(const juce::MouseEvent& event) override;
        void mouseExit(const juce::MouseEvent& event) override;

        void paintSelection(juce::Graphics& g) const;
        void paintFades(juce::Graphics& g) const;

        float ratioForX(int x) const;
        void notifySelectionChanged() const;
        void notifyFadeChanged() const;

        // Ordered selection edges in pixels, valid only while hasSelection_.
        float selectionLeftX() const;
        float selectionRightX() const;

        // The handle whose hit area contains the given local point, or None.
        FadeHandle fadeHandleAt(juce::Point<int> point) const;
        bool fadeHandlesVisible() const;
        void resetFades();

        // The part of the selection the given local point is over, or None. A
        // fade handle sitting over the same point wins, since it is the smaller
        // target and is drawn on top.
        SelectionHit selectionHitAt(juce::Point<int> point) const;

        // The edge the current resize is moving, derived from which side of the
        // anchor the moving end has ended up on.
        SelectionHit resizedEdge() const;

        // Slides the whole selection so the point it was grabbed by follows the
        // pointer, without letting it leave the component or change width.
        void moveSelectionTo(float pointerRatio);

        // Repaints and swaps the mouse cursor when what the pointer is over
        // changes.
        void setHoveredHit(SelectionHit hit);
        juce::MouseCursor cursorFor(SelectionHit hit) const;

        WaveformRenderer renderer_;
        bool selectionEnabled_;
        bool selectionPersistent_;
        float selectionStartRatio_;
        float selectionEndRatio_;
        bool hasSelection_;
        bool fadeEnabled_;
        float fadeInRatio_;
        float fadeOutRatio_;
        FadeHandle activeFadeHandle_;
        SelectionHit hoveredHit_;
        DragMode dragMode_;

        // Captured when a move drag starts: how far into the selection it was
        // grabbed, and the width it keeps for the rest of the drag.
        float moveGrabOffsetRatio_;
        float moveWidthRatio_;

        // Where a press that has not moved yet would anchor a new selection.
        float pendingAnchorRatio_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Waveform)
    };

}
