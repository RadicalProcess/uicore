#pragma once

#include <map>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

namespace rp::uicore
{

    // An 88-key (A0..C8) piano keyboard built on juce::MidiKeyboardComponent.
    //
    // On top of the standard component it adds three things:
    //   * per-key fill colours (setColor / clearColor / clearColors),
    //   * an octave label under every A key (A0, A1, A2, ...),
    //   * a single "selected" key that is drawn with an outline (setSelection).
    //
    // The keyboard shares a juce::MidiKeyboardState with the host, following the
    // usual JUCE idiom; the state must outlive the view.
    //
    // For a keyboard that scrolls across the whole MIDI range with its own scroll
    // bar, use KeyboardView, which wraps this class.
    class Keyboard : public juce::MidiKeyboardComponent
    {
    public:
        explicit Keyboard(juce::MidiKeyboardState& state,
                          Orientation orientation = Orientation::horizontalKeyboard);

        // Sets a custom fill colour for a single key. The colour is used as the
        // key's base colour; the usual mouse-over / key-down overlays are still
        // drawn on top of it.
        void setColor(int midiNoteNumber, juce::Colour colour);

        // Removes the custom colour of a single key, reverting it to the default.
        void clearColor(int midiNoteNumber);

        // Removes every custom key colour.
        void clearColors();

        // Outlines the given key to mark it as the current selection. Only one key
        // can be selected at a time; pass -1 (or any note outside the range) to
        // clear the selection.
        void setSelection(int midiNoteNumber);

        // The currently selected key, or -1 when nothing is selected.
        int getSelection() const noexcept;

    protected:
        void resized() override;
        void drawWhiteNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
                           bool isDown, bool isOver, juce::Colour lineColour, juce::Colour textColour) override;
        void drawBlackNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
                           bool isDown, bool isOver, juce::Colour noteFillColour) override;
        juce::String getWhiteNoteText(int midiNoteNumber) override;

    private:
        // Draws the selection outline inside a key's area.
        void drawSelectionOutline(juce::Graphics& g, const juce::Rectangle<float>& area) const;

        // Returns the custom colour for a key, or an empty optional when none is set.
        const juce::Colour* findColor(int midiNoteNumber) const;

        std::map<int, juce::Colour> keyColours_;
        int selectedNote_ = -1;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Keyboard)
    };

}
