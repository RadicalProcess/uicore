#include <UICore/KeyboardView.h>

namespace rp::uicore
{

    namespace
    {
        // 88-key piano range: A0 (MIDI 21) to C8 (MIDI 108).
        constexpr int kFirstKey = 21;
        constexpr int kLastKey = 108;

        // Middle C = C4 so that A0 (MIDI 21) is labelled "A0".
        constexpr int kOctaveForMiddleC = 4;

        // Semitone offset of the A notes within an octave (C == 0).
        constexpr int kNoteA = 9;

        const juce::Colour kSelectionColour = juce::Colours::red;
    }

    KeyboardView::KeyboardView(juce::MidiKeyboardState& state, Orientation orientation)
    : juce::MidiKeyboardComponent(state, orientation)
    {
        setAvailableRange(kFirstKey, kLastKey);
        setOctaveForMiddleC(kOctaveForMiddleC);
        // Scrolling stays enabled (so setLowestVisibleKey is honoured and callers
        // can drive it from an external scroll bar), but the built-in octave
        // scroll buttons are hidden in resized(). Calling setScrollButtonsVisible
        // (false) here would instead disable scrolling entirely: the base class
        // snaps the view back to the first key on every layout when it cannot
        // scroll.
    }

    void KeyboardView::resized()
    {
        juce::MidiKeyboardComponent::resized();

        // The base class lays out (and shows) its two octave scroll buttons here;
        // they are this component's only child components. Hide them so the keys
        // span the full width and scrolling is left to an external control.
        for (auto* child : getChildren())
        {
            if (auto* button = dynamic_cast<juce::Button*>(child))
                button->setVisible(false);
        }
    }

    void KeyboardView::setColor(int midiNoteNumber, juce::Colour colour)
    {
        keyColours_[midiNoteNumber] = colour;
        repaint();
    }

    void KeyboardView::clearColor(int midiNoteNumber)
    {
        keyColours_.erase(midiNoteNumber);
        repaint();
    }

    void KeyboardView::clearColors()
    {
        keyColours_.clear();
        repaint();
    }

    void KeyboardView::setSelection(int midiNoteNumber)
    {
        if (selectedNote_ == midiNoteNumber)
            return;

        selectedNote_ = midiNoteNumber;
        repaint();
    }

    int KeyboardView::getSelection() const noexcept
    {
        return selectedNote_;
    }

    void KeyboardView::drawWhiteNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
                                     bool isDown, bool isOver, juce::Colour lineColour, juce::Colour textColour)
    {
        // The base implementation only overlays the down / over states on top of
        // the component background, so filling the custom colour first lets it
        // show through as the key's base colour.
        if (const auto* colour = findColor(midiNoteNumber))
        {
            g.setColour(*colour);
            g.fillRect(area);
        }

        juce::MidiKeyboardComponent::drawWhiteNote(midiNoteNumber, g, area, isDown, isOver, lineColour, textColour);

        if (midiNoteNumber == selectedNote_)
            drawSelectionOutline(g, area);
    }

    void KeyboardView::drawBlackNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
                                     bool isDown, bool isOver, juce::Colour noteFillColour)
    {
        const auto* colour = findColor(midiNoteNumber);
        const auto fillColour = colour != nullptr ? *colour : noteFillColour;

        juce::MidiKeyboardComponent::drawBlackNote(midiNoteNumber, g, area, isDown, isOver, fillColour);

        if (midiNoteNumber == selectedNote_)
            drawSelectionOutline(g, area);
    }

    juce::String KeyboardView::getWhiteNoteText(int midiNoteNumber)
    {
        if (midiNoteNumber % 12 == kNoteA)
            return juce::MidiMessage::getMidiNoteName(midiNoteNumber, true, true, getOctaveForMiddleC());

        return {};
    }

    void KeyboardView::drawSelectionOutline(juce::Graphics& g, const juce::Rectangle<float>& area) const
    {
        const auto outline = area.reduced(1.0f);
        g.setColour(kSelectionColour);
        g.drawRect(outline, 2.0f);
    }

    const juce::Colour* KeyboardView::findColor(int midiNoteNumber) const
    {
        const auto it = keyColours_.find(midiNoteNumber);
        if (it == keyColours_.end())
            return nullptr;

        return &it->second;
    }

}
