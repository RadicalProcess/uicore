#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <UICore/Keyboard.h>

namespace rp::uicore
{

    // A scrollable piano keyboard: it wraps a Keyboard that spans the whole MIDI
    // range (0..127) but only shows a four-octave window (A3..A7) at a time, with
    // a horizontal scroll bar under the keys to pan left and right.
    //
    // This is the component to reach for by default; use the wrapped Keyboard
    // directly (via keyboard()) to colour keys or set the selection.
    //
    // The keyboard shares a juce::MidiKeyboardState with the host; the state must
    // outlive the view.
    class KeyboardView : public juce::Component,
                         private juce::ScrollBar::Listener,
                         private juce::ChangeListener
    {
    public:
        explicit KeyboardView(juce::MidiKeyboardState& state);
        ~KeyboardView() override;

        // The wrapped keys, for colouring / selection and any other Keyboard API.
        Keyboard& keyboard() noexcept;
        const Keyboard& keyboard() const noexcept;

        // The scroll bar under the keys, so a host can restyle it through the
        // usual juce::ScrollBar ColourIds.
        juce::ScrollBar& scrollBar() noexcept;

        void resized() override;

    private:
        // Keeps the scroll bar's thumb in step with the keys' scroll position
        // (e.g. after a mouse-wheel scroll).
        void changeListenerCallback(juce::ChangeBroadcaster* source) override;

        // Pans the keys when the user drags the scroll bar.
        void scrollBarMoved(juce::ScrollBar* scrollBarThatHasMoved, double newRangeStart) override;

        Keyboard keys_;
        juce::ScrollBar scrollBar_{false};

        // Guards against the keys <-> scroll bar updates feeding back on each other.
        bool ignoreScrollCallbacks_ = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KeyboardView)
    };

}
