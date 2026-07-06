#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <UICore/IconButton.h>

#include <functional>

namespace rp::uicore
{
    // A horizontal transport strip carrying the playback controls. The
    // play/pause toggle and the stop button are tucked into the left corner;
    // both are round and scale to the strip's height.
    //
    // Play/pause is a toggle whose glyph swaps with its state; stop is
    // momentary. Each fires the matching callback when clicked.
    class TransportView : public juce::Component
    {
    public:
        TransportView();
        ~TransportView() override = default;

        void paint(juce::Graphics& g) override;
        void resized() override;

        // Fired when the play/pause button is toggled. The argument is the new
        // toggle state: true means playing, false means paused.
        std::function<void(bool)> onPlayPause;

        // Fired when the stop button is pressed.
        std::function<void()> onStop;

    private:
        PlayPauseButton playPauseButton_;
        StopButton stopButton_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportView)
    };
}
