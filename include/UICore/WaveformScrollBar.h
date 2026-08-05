#pragma once

#include "WaveformRenderer.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <vector>

namespace rp::uicore
{

    // A scrollbar whose track is a waveform: the whole sound drawn small, with
    // the slice a companion view is showing marked on it as a thumb. It is how
    // the user travels a sound too long to fit at a useful zoom, and how they
    // see where they are once zoomed in.
    //
    // It carries no selection and edits nothing. A second range can be marked on
    // it for reference — typically whatever the companion view is editing — so
    // that a range running off both sides of that view can still be seen whole
    // here. Drag the thumb to travel, drag its edges to change how much is on
    // show, or click the track to jump. Every colour comes from a ColourId so a
    // host can tell the thumb and the marked range apart.
    class WaveformScrollBar : public juce::Component
    {
    public:
        enum ColourIds
        {
            backgroundColourId = 0x2007000,
            outlineColourId = 0x2007001,
            traceColourId = 0x2007002,
            thumbColourId = 0x2007003,
            markerColourId = 0x2007004
        };

        WaveformScrollBar();
        ~WaveformScrollBar() override = default;

        void setWaveformData(const std::vector<std::vector<float>>& waveformData);

        // The slice on show, as ratios of the whole sound. Clamped and ordered,
        // and never narrower than a thumb the user could still grab.
        void setView(float startRatio, float endRatio);

        float getViewStart() const;

        float getViewEnd() const;

        // A range drawn for reference only, as ratios of the whole sound. Pass
        // an empty range to mark nothing.
        void setMarkedRange(float startRatio, float endRatio);

        // Invoked whenever the user moves the thumb, both during the drag and
        // once the mouse is released. Never invoked by setView.
        std::function<void(float startRatio, float endRatio)> onViewChanged;

    private:
        // Which part of the thumb, if any, a point is over: an edge, which a
        // drag resizes, or the body, which a drag slides.
        enum class ThumbHit
        {
            None,
            LeftEdge,
            RightEdge,
            Body
        };

        void paint(juce::Graphics& g) override;

        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;
        void mouseMove(const juce::MouseEvent& event) override;
        void mouseExit(const juce::MouseEvent& event) override;

        float ratioForX(int x) const;
        float thumbLeftX() const;
        float thumbRightX() const;

        ThumbHit thumbHitAt(juce::Point<int> point) const;
        void setHoveredHit(ThumbHit hit);
        juce::MouseCursor cursorFor(ThumbHit hit) const;

        // Places the view from a pointer position, according to what the drag
        // grabbed, and tells the host.
        void dragTo(float pointerRatio);
        void placeView(float start, float width);
        void notifyViewChanged() const;

        WaveformRenderer renderer_;
        float viewStartRatio_;
        float viewEndRatio_;
        float markedStartRatio_;
        float markedEndRatio_;
        ThumbHit hoveredHit_;
        ThumbHit draggedHit_;

        // How far into the thumb a body drag grabbed it, and the width that drag
        // keeps.
        float grabOffsetRatio_;
        float grabWidthRatio_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformScrollBar)
    };

}
