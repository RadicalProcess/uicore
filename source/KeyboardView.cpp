#include <UICore/KeyboardView.h>

#include <UICore/Style.h>

namespace rp::uicore
{

    namespace
    {
        // Visible window: four octaves from A3 (MIDI 57) to A7 (MIDI 105).
        constexpr int kLowestVisibleNote = 57;
        constexpr int kHighestVisibleNote = 105;

        // White keys spanning A3..A7 inclusive; drives the key width so exactly the
        // four-octave window fits the keyboard's width.
        constexpr int kVisibleWhiteKeys = 29;

        // The keyboard can be scrolled across the whole MIDI range.
        constexpr int kFirstMidiNote = 0;
        constexpr int kLastMidiNote = 127;

        constexpr int kScrollBarHeight = 16;
    }

    KeyboardView::KeyboardView(juce::MidiKeyboardState& state)
    : keys_(state)
    {
        keys_.setAvailableRange(kFirstMidiNote, kLastMidiNote);
        keys_.setLowestVisibleKey(kLowestVisibleNote);
        keys_.addChangeListener(this);
        addAndMakeVisible(keys_);

        const auto visibleSpan = kHighestVisibleNote - kLowestVisibleNote + 1;
        scrollBar_.setRangeLimits(kFirstMidiNote, kLastMidiNote + 1);
        scrollBar_.setCurrentRange(kLowestVisibleNote, visibleSpan, juce::dontSendNotification);
        scrollBar_.setAutoHide(false);
        scrollBar_.setColour(juce::ScrollBar::thumbColourId, styles::highlight);
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

    void KeyboardView::resized()
    {
        auto area = getLocalBounds();
        scrollBar_.setBounds(area.removeFromBottom(kScrollBarHeight));

        keys_.setBounds(area);
        keys_.setKeyWidth(static_cast<float>(area.getWidth()) / static_cast<float>(kVisibleWhiteKeys));
    }

    void KeyboardView::changeListenerCallback(juce::ChangeBroadcaster* source)
    {
        if (source != &keys_ || ignoreScrollCallbacks_)
            return;

        const juce::ScopedValueSetter<bool> guard(ignoreScrollCallbacks_, true);
        scrollBar_.setCurrentRangeStart(keys_.getLowestVisibleKey(), juce::dontSendNotification);
    }

    void KeyboardView::scrollBarMoved(juce::ScrollBar* scrollBarThatHasMoved, double newRangeStart)
    {
        if (scrollBarThatHasMoved != &scrollBar_ || ignoreScrollCallbacks_)
            return;

        const juce::ScopedValueSetter<bool> guard(ignoreScrollCallbacks_, true);
        keys_.setLowestVisibleKey(juce::roundToInt(newRangeStart));
    }

}
