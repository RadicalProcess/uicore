#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <vector>

namespace rp::uicore
{

    // Renders an audio waveform, and an optional playhead, into a rectangle. The
    // drawing automatically switches between three mutually exclusive
    // representations as the audio gets denser: a connected line with a dot per
    // sample, a connected line only, and peak vertical lines (the
    // samples-per-pixel thresholds live in the .cpp). Mono data fills the whole
    // rectangle; stereo data is split into an upper and a lower half.
    //
    // The renderer owns the waveform data and playhead state but paints neither
    // a background nor a border, so it can serve both as the body of the
    // Waveform component and as a translucent backdrop behind the MotionView
    // curve. Callers pass the colours, so a low-alpha colour yields a faint
    // rendering.
    class WaveformRenderer
    {
    public:
        WaveformRenderer();

        void setWaveformData(const std::vector<std::vector<float>>& waveformData);

        bool isEmpty() const;

        void setPlayheadPosition(float positionRatio);

        void setPlayheadVisibility(bool visible);

        bool isPlayheadVisible() const;

        // Draws the waveform across bounds in the given colour. Pass a colour
        // with reduced alpha for a translucent rendering.
        void paintWaveform(juce::Graphics& g, const juce::Rectangle<float>& bounds, const juce::Colour& colour) const;

        // Draws the waveform along an arbitrary path instead of across a
        // rectangle: the first channel is walked as evenly spaced peak lines
        // (the same peak-bucket rendering the rectangular version uses for dense
        // audio), each drawn perpendicular to the path with its half-length set
        // by amplitude times the bucket peak, so the waveform follows the curve.
        // Pass a colour with reduced alpha for a translucent rendering.
        void paintWaveformAlongPath(juce::Graphics& g, const juce::Path& path, float amplitude, const juce::Colour& colour) const;

        // Draws the playhead as a vertical line at the current position across
        // bounds. Does nothing while the playhead is hidden.
        void paintPlayhead(juce::Graphics& g, const juce::Rectangle<float>& bounds, const juce::Colour& colour) const;

    private:
        std::vector<std::vector<float>> waveformData_;
        float playheadPositionRatio_;
        bool playheadVisibility_;
    };

}
