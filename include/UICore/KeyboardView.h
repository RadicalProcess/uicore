#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <UICore/Keyboard.h>

namespace rp::uicore
{

    // A scrollable piano keyboard: it wraps a Keyboard that spans the whole MIDI
    // range (0..127) but only shows a four-octave window (C2..B5 to begin with)
    // at a time, with a horizontal scroll bar under the keys to pan left and
    // right.
    //
    // This is the component to reach for by default; use the wrapped Keyboard
    // directly (via keyboard()) to colour keys or set the selection.
    //
    // The window is always exactly four octaves wide, so a host can drive it from
    // a range control of its own: read it through getLowestVisibleKey() /
    // getHighestVisibleKey(), move it with setLowestVisibleKey(), follow it by
    // registering as a juce::ChangeListener, and hide the built-in scroll bar
    // with setScrollBarVisible(false).
    //
    // The keyboard shares a juce::MidiKeyboardState with the host; the state must
    // outlive the view.
    class KeyboardView : public juce::Component,
                         public juce::ChangeBroadcaster,
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

        // Shows or hides the scroll bar under the keys. Hiding it gives the keys
        // its height as well, for a host that pans the keyboard itself.
        void setScrollBarVisible(bool visible);

        // The MIDI notes at the two ends of the visible window, inclusive.
        int getLowestVisibleKey() const noexcept;
        int getHighestVisibleKey() const noexcept;

        // Pans the keys so the window starts at the given note, clamped so the
        // whole four-octave window stays inside the MIDI range. Listeners are
        // told about the move like any other scroll.
        void setLowestVisibleKey(int midiNoteNumber);

        void resized() override;

    private:
        // Keeps the scroll bar's thumb in step with the keys' scroll position
        // (e.g. after a mouse-wheel scroll), and tells listeners the window moved.
        void changeListenerCallback(juce::ChangeBroadcaster* source) override;

        // Pans the keys when the user drags the scroll bar.
        void scrollBarMoved(juce::ScrollBar* scrollBarThatHasMoved, double newRangeStart) override;

        Keyboard keys_;
        juce::ScrollBar scrollBar_{false};

        // Guards against the keys <-> scroll bar updates feeding back on each other.
        bool ignoreScrollCallbacks_ = false;

        // The window start listeners were last told about, so a scroll that ends
        // where it began stays silent.
        int notifiedLowestKey_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KeyboardView)
    };

}
