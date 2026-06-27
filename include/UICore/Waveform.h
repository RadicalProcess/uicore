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
    // editor view. Selection is opt-in via setSelectionEnabled.
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

        // Invoked whenever the selection changes, either through user
        // interaction or setSelection / clearSelection.
        std::function<void(float startRatio, float endRatio)> onSelectionChanged;

    private:
        void paint(juce::Graphics& g) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;

        void paintWaveform(juce::Graphics& g);
        void paintPlaybackPosition(juce::Graphics& g) const;
        void paintSelection(juce::Graphics& g) const;

        float ratioForX(int x) const;
        void notifySelectionChanged() const;

        std::vector<std::vector<float>> waveformData_;
        float playheadPositionRatio_;
        bool playheadVisibility_;
        bool selectionEnabled_;
        float selectionStartRatio_;
        float selectionEndRatio_;
        bool hasSelection_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Waveform)
    };

}
