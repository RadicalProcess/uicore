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
        setScrollButtonsVisible(false);
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
