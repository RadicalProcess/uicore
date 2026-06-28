#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <vector>

namespace rp::uicore
{

    // Displays a (potentially very long) sample. In addition to a playhead, the
    // user can select a region by clicking and dragging; the selected region is
    // highlighted with a translucent orange overlay and reported back as a pair
    // of normalised ratios (0..1) so a caller can show that slice in another
    // editor view. Selection is opt-in via setSelectionEnabled. The selection
    // can additionally carry a fade-in and fade-out, edited via draggable
    // triangle handles and opt-in through setFadeEnabled.
    class Waveform : public juce::Component
    {
    public:
        Waveform();
        ~Waveform() override = default;

        void setWaveformData(const std::vector<std::vector<float>>& waveformData);

        void setPlayheadPosition(float positionRatio);

        void setPlayheadVisibility(bool visible);

        // Enables click-and-drag region selection. Disabled by default; when
        // disabled the component ignores mouse interaction and any existing
        // selection is cleared.
        void setSelectionEnabled(bool enabled);

        // Programmatically set the selection. Both arguments are ratios in the
        // range 0..1; they are clamped and ordered so that start <= end. Has no
        // effect while selection is disabled (see setSelectionEnabled).
        void setSelection(float startRatio, float endRatio);

        void clearSelection();

        bool hasSelection() const;

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

        void paint(juce::Graphics& g) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;
        void mouseMove(const juce::MouseEvent& event) override;
        void mouseExit(const juce::MouseEvent& event) override;

        void paintWaveform(juce::Graphics& g);
        void paintPlaybackPosition(juce::Graphics& g) const;
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

        std::vector<std::vector<float>> waveformData_;
        float playheadPositionRatio_;
        bool playheadVisibility_;
        bool selectionEnabled_;
        float selectionStartRatio_;
        float selectionEndRatio_;
        bool hasSelection_;
        bool fadeEnabled_;
        float fadeInRatio_;
        float fadeOutRatio_;
        FadeHandle activeFadeHandle_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Waveform)
    };

}
