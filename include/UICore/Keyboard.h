#pragma once

#include <map>
#include <set>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

namespace rp::uicore
{

    // An 88-key (A0..C8) piano keyboard built on juce::MidiKeyboardComponent.
    //
    // On top of the standard component it adds four things:
    //   * per-key fill colours (setColor / clearColor / clearColors),
    //   * an octave label under every C key (C0, C1, C2, ...),
    //   * a single "selected" key that is drawn with an outline (setSelection),
    //   * a "playing" overlay wash on any number of keys (setPlaying), layered
    //     on top of the fill colour and independent of it, so callers driving
    //     transient playback feedback never disturb the persistent fill colour.
    //
    // The keys are drawn flat: a black note is a plain filled rectangle with
    // none of juce::MidiKeyboardComponent's bevel highlight, so the keyboard
    // sits in a flat-design host without a raised, three-dimensional look.
    //
    // The keyboard shares a juce::MidiKeyboardState with the host, following the
    // usual JUCE idiom; the state must outlive the view.
    //
    // For a keyboard that scrolls across the whole MIDI range with its own scroll
    // bar, use KeyboardView, which wraps this class.
    class Keyboard : public juce::MidiKeyboardComponent
    {
    public:
        // The selection outline and the playing wash on top of the ColourIds
        // juce::MidiKeyboardComponent already defines for the keys themselves.
        enum ColourIds
        {
            selectionColourId = 0x2003000,
            playingOverlayColourId = 0x2003001
        };

        explicit Keyboard(juce::MidiKeyboardState& state,
                          Orientation orientation = Orientation::horizontalKeyboard);

        // Sets a custom fill colour for a single key. The colour is composited
        // over the key's own base colour, so a translucent colour tints a white
        // and a black key alike; the usual mouse-over / key-down overlays are
        // still drawn on top of it.
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

        // Marks a key as currently playing (or not), drawing a translucent
        // overlay on top of the key's usual fill colour. Independent of
        // setColor/clearColor, so it never overwrites the persistent fill.
        void setPlaying(int midiNoteNumber, bool playing);

    protected:
        void resized() override;
        void drawWhiteNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
                           bool isDown, bool isOver, juce::Colour lineColour, juce::Colour textColour) override;
        void drawBlackNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
                           bool isDown, bool isOver, juce::Colour noteFillColour) override;
        juce::String getWhiteNoteText(int midiNoteNumber) override;

    private:
        // Fills a key's area with its base colour, tinted by the custom colour
        // set for that key, if any.
        void fillKey(juce::Graphics& g, const juce::Rectangle<float>& area, int midiNoteNumber,
                     juce::Colour baseColour) const;

        // Draws the selection outline inside a key's area.
        void drawSelectionOutline(juce::Graphics& g, const juce::Rectangle<float>& area) const;

        // Draws the playing overlay wash inside a key's area, if applicable.
        void drawPlayingOverlay(int midiNoteNumber, juce::Graphics& g, const juce::Rectangle<float>& area) const;

        // Returns the custom colour for a key, or an empty optional when none is set.
        const juce::Colour* findColor(int midiNoteNumber) const;

        std::map<int, juce::Colour> keyColours_;
        int selectedNote_ = -1;
        std::set<int> playingNotes_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Keyboard)
    };

}
