#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <vector>

namespace rp::uicore
{

    // Displays a (potentially very long) sample and lets the user select a
    // region by clicking and dragging. The selected region is highlighted with
    // a translucent orange overlay and reported back as a pair of normalised
    // ratios (0..1) so a caller can show that slice in another editor view.
    class WaveformSelector : public juce::Component
    {
    public:
        WaveformSelector();
        ~WaveformSelector() override = default;

        void setWaveformData(const std::vector<std::vector<float>>& waveformData);

        // Programmatically set the selection. Both arguments are ratios in the
        // range 0..1; they are clamped and ordered so that start <= end.
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
        void paintSelection(juce::Graphics& g) const;

        float ratioForX(int x) const;
        void notifySelectionChanged() const;

        std::vector<std::vector<float>> waveformData_;
        float selectionStartRatio_;
        float selectionEndRatio_;
        bool hasSelection_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformSelector)
    };

}
