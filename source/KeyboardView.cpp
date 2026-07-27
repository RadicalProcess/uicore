#include <UICore/KeyboardView.h>

#include <UICore/Style.h>

namespace rp::uicore
{

    namespace
    {
        // Visible window: four octaves, starting at C2 (MIDI 36).
        constexpr int kLowestVisibleNote = 36;
        constexpr int kVisibleNotes = 48;

        // Any four-octave window holds exactly four times the seven white keys of
        // an octave; this drives the key width so the window fits the width.
        constexpr int kVisibleWhiteKeys = 28;

        // The keyboard can be scrolled across the whole MIDI range.
        constexpr int kFirstMidiNote = 0;
        constexpr int kLastMidiNote = 127;

        constexpr int kScrollBarHeight = 16;

        int clampedLowestNote(int midiNoteNumber)
        {
            return juce::jlimit(kFirstMidiNote, kLastMidiNote - kVisibleNotes + 1, midiNoteNumber);
        }
    }

    KeyboardView::KeyboardView(juce::MidiKeyboardState& state)
    : keys_(state)
    , notifiedLowestKey_(kLowestVisibleNote)
    {
        keys_.setAvailableRange(kFirstMidiNote, kLastMidiNote);
        keys_.setLowestVisibleKey(kLowestVisibleNote);
        keys_.addChangeListener(this);
        addAndMakeVisible(keys_);

        scrollBar_.setRangeLimits(kFirstMidiNote, kLastMidiNote + 1);
        scrollBar_.setCurrentRange(kLowestVisibleNote, kVisibleNotes, juce::dontSendNotification);
        scrollBar_.setAutoHide(false);
        scrollBar_.addListener(this);
        addAndMakeVisible(scrollBar_);
    }

    KeyboardView::~KeyboardView()
    {
        keys_.removeChangeListener(this);
    }

    Keyboard& KeyboardView::keyboard() noexcept
    {
        return keys_;
    }

    const Keyboard& KeyboardView::keyboard() const noexcept
    {
        return keys_;
    }

    juce::ScrollBar& KeyboardView::scrollBar() noexcept
    {
        return scrollBar_;
    }

    void KeyboardView::setScrollBarVisible(bool visible)
    {
        scrollBar_.setVisible(visible);
        resized();
    }

    int KeyboardView::getLowestVisibleKey() const noexcept
    {
        return keys_.getLowestVisibleKey();
    }

    int KeyboardView::getHighestVisibleKey() const noexcept
    {
        return keys_.getLowestVisibleKey() + kVisibleNotes - 1;
    }

    void KeyboardView::setLowestVisibleKey(int midiNoteNumber)
    {
        keys_.setLowestVisibleKey(clampedLowestNote(midiNoteNumber));
    }

    void KeyboardView::resized()
    {
        auto area = getLocalBounds();

        if (scrollBar_.isVisible())
            scrollBar_.setBounds(area.removeFromBottom(kScrollBarHeight));

        keys_.setBounds(area);
        keys_.setKeyWidth(static_cast<float>(area.getWidth()) / static_cast<float>(kVisibleWhiteKeys));
    }

    void KeyboardView::changeListenerCallback(juce::ChangeBroadcaster* source)
    {
        if (source != &keys_)
            return;

        if (!ignoreScrollCallbacks_)
        {
            const juce::ScopedValueSetter<bool> guard(ignoreScrollCallbacks_, true);
            scrollBar_.setCurrentRangeStart(keys_.getLowestVisibleKey(), juce::dontSendNotification);
        }

        if (keys_.getLowestVisibleKey() == notifiedLowestKey_)
            return;

        notifiedLowestKey_ = keys_.getLowestVisibleKey();
        sendChangeMessage();
    }

    void KeyboardView::scrollBarMoved(juce::ScrollBar* scrollBarThatHasMoved, double newRangeStart)
    {
        if (scrollBarThatHasMoved != &scrollBar_ || ignoreScrollCallbacks_)
            return;

        const juce::ScopedValueSetter<bool> guard(ignoreScrollCallbacks_, true);
        keys_.setLowestVisibleKey(clampedLowestNote(juce::roundToInt(newRangeStart)));
    }

}
