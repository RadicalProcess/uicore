#pragma once

#include "WaveformRenderer.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <vector>

namespace rp::uicore
{

    // An editor for a segmented motion curve: a polyline through a set of nodes
    // drawn left to right. Each node is a draggable square handle and the
    // segments between them are lines that can be bent into power curves. The
    // curve always spans the full width because the leftmost and rightmost nodes
    // are pinned to the side edges and cannot be removed; they may only be moved
    // vertically. Interior nodes are added by clicking on an empty spot, moved by
    // dragging, and removed by shift-clicking. The default content is a single
    // segment running from the bottom-left corner to the top-right corner.
    //
    // A segment highlights while the cursor hovers over it. Clicking on a segment
    // and dragging up or down bends it into a symmetric bow in that direction;
    // shift-clicking a segment straightens it again.
    //
    // Nodes are exposed as points normalised to 0..1, with x increasing to the
    // right and y increasing upwards (so y = 0 is the bottom edge and y = 1 the
    // top edge), always sorted by ascending x. Each segment also carries a signed
    // bend in -1..1, where 0 is straight, positive bows the segment upwards and
    // negative downwards.
    class MotionView : public juce::Component
    {
    public:
        MotionView();
        ~MotionView() override = default;

        std::vector<juce::Point<float>> getPoints() const;

        // The bend of every segment, one entry per gap between consecutive nodes
        // (so the size is always getPoints().size() - 1). Each value is in -1..1.
        std::vector<float> getBends() const;

        // Replaces the curve. The points are clamped to 0..1 and sorted by x;
        // the first and last become the pinned endpoints. Fewer than two points
        // is treated as a request to restore the default single segment. All
        // segments are reset to straight.
        void setPoints(const std::vector<juce::Point<float>>& points);

        // Restores the default single straight segment from the bottom-left
        // corner to the top-right corner.
        void reset();

        // Shows an audio waveform as a faint backdrop behind the curve, drawn in
        // the same plot area so it shares the node coordinate space. Passing
        // empty data hides the backdrop. The rendering matches the Waveform
        // component (see WaveformRenderer).
        void setWaveformData(const std::vector<std::vector<float>>& waveformData);

        // Position (0..1) and visibility of an optional playhead drawn over the
        // backdrop. Hidden by default.
        void setPlayheadPosition(float positionRatio);

        void setPlayheadVisibility(bool visible);

        // Sets thin horizontal reference lines drawn behind the curve as visual
        // guides. Each position is a normalised y value in 0..1 (0 = bottom edge,
        // 1 = top edge); values are clamped to that range. Every line carries a
        // numbered label on its left, numbered from 1 in the order given. An
        // empty vector removes all reference lines.
        void setReferenceLines(const std::vector<float>& linePositions);

        // Invoked whenever the curve changes, either through user interaction or
        // setPoints / reset.
        std::function<void(const std::vector<juce::Point<float>>& points)> onChange;

    private:
        void paint(juce::Graphics& g) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;
        void mouseMove(const juce::MouseEvent& event) override;
        void mouseExit(const juce::MouseEvent& event) override;

        // The rectangle the curve is plotted into, inset from the bounds so the
        // node handles at the edges are never clipped.
        juce::Rectangle<float> plotArea() const;

        // Pixel position of a normalised node, and the inverse mapping of a pixel
        // position back to a normalised point clamped to 0..1.
        juce::Point<float> toPixel(juce::Point<float> normalised) const;
        juce::Point<float> toNormalised(juce::Point<float> pixel) const;

        // Pixel position along segment i at the given fraction (0..1) of its
        // width, following the segment's bend.
        juce::Point<float> pointOnSegment(int segment, float fraction) const;

        // Appends segment i to the path, sampling its bow curve when bent.
        void buildSegmentPath(juce::Path& path, int segment) const;

        // Index of the node whose handle contains the given local point, or -1.
        int nodeAt(juce::Point<float> point) const;

        // Index of the segment whose (possibly bent) line passes near the given
        // local point, or -1.
        int segmentAt(juce::Point<float> point) const;

        // Endpoints are pinned and cannot be removed.
        bool isEndpoint(int index) const;

        void notifyChange() const;

        std::vector<juce::Point<float>> points_;

        // Normalised y positions (0..1) of the horizontal reference lines.
        std::vector<float> referenceLines_;

        // Signed bend of each segment in -1..1; always points_.size() - 1 long.
        std::vector<float> bends_;

        // Index of the node currently being dragged, or -1 when idle.
        int draggedIndex_;

        // Index of the segment currently being bent, or -1 when idle.
        int bentIndex_;

        // Index of the segment the cursor is hovering over, or -1 when none.
        int hoveredIndex_;

        // Mouse y (pixels) and the segment's bend when the current bend drag
        // started, used to turn vertical drag distance into a change in bend.
        float bendDragStartY_;
        float bendDragStartBend_;

        // Faint waveform (and optional playhead) drawn behind the curve.
        WaveformRenderer waveformRenderer_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MotionView)
    };

}
