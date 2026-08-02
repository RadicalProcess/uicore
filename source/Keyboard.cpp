#include <UICore/Keyboard.h>
#include <UICore/Style.h>

namespace rp::uicore
{

    namespace
    {
        // 88-key piano range: A0 (MIDI 21) to C8 (MIDI 108).
        constexpr int kFirstKey = 21;
        constexpr int kLastKey = 108;

        // Middle C = C4 so that A0 (MIDI 21) is labelled "A0".
        constexpr int kOctaveForMiddleC = 4;

        // Semitone offset of the C notes within an octave (C == 0).
        constexpr int kNoteC = 0;

        // Diameter of the marker dot, and the room left between it and the
        // bottom edge of the key.
        constexpr float kMarkerDiameter = 8.0f;
        constexpr float kMarkerBottomMargin = 3.0f;

        // Room left above an octave label, and to its left.
        constexpr float kLabelTopInset = 2.0f;
        constexpr float kLabelLeftInset = 4.0f;
    }

    Keyboard::Keyboard(juce::MidiKeyboardState& state, Orientation orientation)
        : juce::MidiKeyboardComponent(state, orientation)
    {
        setAvailableRange(kFirstKey, kLastKey);
        setOctaveForMiddleC(kOctaveForMiddleC);
        setColour(selectionColourId, juce::Colours::red);
        setColour(playingOverlayColourId, styles::playing.withAlpha(0.55f));
        // Scrolling stays enabled (so setLowestVisibleKey is honoured and callers
        // can drive it from an external scroll bar), but the built-in octave
        // scroll buttons are hidden in resized(). Calling setScrollButtonsVisible
        // (false) here would instead disable scrolling entirely: the base class
        // snaps the view back to the first key on every layout when it cannot
        // scroll.
    }

    void Keyboard::resized()
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

    void Keyboard::setColor(int midiNoteNumber, juce::Colour colour)
    {
        keyColours_[midiNoteNumber] = colour;
        repaint();
    }

    void Keyboard::clearColor(int midiNoteNumber)
    {
        keyColours_.erase(midiNoteNumber);
        repaint();
    }

    void Keyboard::clearColors()
    {
        keyColours_.clear();
        repaint();
    }

    void Keyboard::setMarker(int midiNoteNumber, juce::Colour colour)
    {
        keyMarkers_[midiNoteNumber] = colour;
        repaint();
    }

    void Keyboard::clearMarker(int midiNoteNumber)
    {
        keyMarkers_.erase(midiNoteNumber);
        repaint();
    }

    void Keyboard::clearMarkers()
    {
        keyMarkers_.clear();
        repaint();
    }

    void Keyboard::setSelection(int midiNoteNumber)
    {
        if (selectedNote_ == midiNoteNumber)
            return;

        selectedNote_ = midiNoteNumber;
        repaint();
    }

    int Keyboard::getSelection() const noexcept
    {
        return selectedNote_;
    }

    void Keyboard::setPlaying(int midiNoteNumber, bool playing)
    {
        const auto changed =
            playing ? playingNotes_.insert(midiNoteNumber).second : playingNotes_.erase(midiNoteNumber) > 0;
        if (changed)
            repaint();
    }

    void Keyboard::drawWhiteNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area, bool isDown,
                                 bool isOver, juce::Colour lineColour, juce::Colour textColour)
    {
        // The base implementation paints the white keys as one background fill
        // and only overlays the down / over states per key, so the key's own
        // colour has to be laid down here for a custom colour to tint it.
        fillKey(g, area, midiNoteNumber, findColour(whiteNoteColourId));

        // The base class draws the octave label at the bottom of the key, where
        // the marker dot now sits, and offers no say in that. It is handed a
        // transparent text colour so its label falls away, and the label is
        // drawn here instead, at the top of the key.
        juce::MidiKeyboardComponent::drawWhiteNote(midiNoteNumber, g, area, isDown, isOver, lineColour,
                                                   juce::Colours::transparentBlack);
        drawOctaveLabel(midiNoteNumber, g, area, textColour);

        if (midiNoteNumber == selectedNote_)
            drawSelectionOutline(g, area);

        drawPlayingOverlay(midiNoteNumber, g, area);
        drawMarker(midiNoteNumber, g, area);
    }

    void Keyboard::drawBlackNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area, bool isDown,
                                 bool isOver, juce::Colour noteFillColour)
    {
        // Deliberately not the base implementation: that one draws a brighter
        // inset band over the key to fake a bevel, which reads as raised. A
        // black key here is one flat rectangle, with the same down / over
        // overlays the white keys get.
        fillKey(g, area, midiNoteNumber, noteFillColour);
        drawPressOverlay(g, area, isDown, isOver);

        if (midiNoteNumber == selectedNote_)
            drawSelectionOutline(g, area);

        drawPlayingOverlay(midiNoteNumber, g, area);
        drawMarker(midiNoteNumber, g, area);
    }

    juce::String Keyboard::getWhiteNoteText(int midiNoteNumber)
    {
        if (midiNoteNumber % 12 == kNoteC)
            return juce::MidiMessage::getMidiNoteName(midiNoteNumber, true, true, getOctaveForMiddleC());

        return {};
    }

    void Keyboard::fillKey(juce::Graphics& g, const juce::Rectangle<float>& area, int midiNoteNumber,
                           juce::Colour baseColour) const
    {
        const auto* colour = findColor(midiNoteNumber);
        g.setColour(colour != nullptr ? baseColour.overlaidWith(*colour) : baseColour);
        g.fillRect(area);
    }

    void Keyboard::drawSelectionOutline(juce::Graphics& g, const juce::Rectangle<float>& area) const
    {
        const auto outline = area.reduced(1.0f);
        g.setColour(findColour(selectionColourId));
        g.drawRect(outline, 2.0f);
    }

    void Keyboard::drawPlayingOverlay(int midiNoteNumber, juce::Graphics& g, const juce::Rectangle<float>& area) const
    {
        if (playingNotes_.find(midiNoteNumber) == playingNotes_.end())
            return;

        g.setColour(findColour(playingOverlayColourId));
        g.fillRect(area);
    }

    void Keyboard::drawPressOverlay(juce::Graphics& g, const juce::Rectangle<float>& area, bool isDown,
                                    bool isOver) const
    {
        auto overlay = juce::Colours::transparentBlack;
        if (isDown)
            overlay = findColour(keyDownOverlayColourId);
        if (isOver)
            overlay = overlay.overlaidWith(findColour(mouseOverKeyOverlayColourId));

        g.setColour(overlay);
        g.fillRect(area);
    }

    void Keyboard::drawOctaveLabel(int midiNoteNumber, juce::Graphics& g, const juce::Rectangle<float>& area,
                                   juce::Colour textColour)
    {
        const auto text = juce::String(getWhiteNoteText(midiNoteNumber));
        if (text.isEmpty() || getOrientation() != horizontalKeyboard)
            return;

        // The font the base class would have labelled the key with.
        const auto fontHeight = juce::jmin(12.0f, getKeyWidth() * 0.9f);

        g.setColour(textColour);
        g.setFont(juce::Font(juce::FontOptions{fontHeight}).withHorizontalScale(0.8f));
        // Left, not centred: the black key overlapping the upper right of a C
        // key would otherwise cover the label.
        g.drawText(text, area.withTrimmedLeft(kLabelLeftInset).withTrimmedTop(kLabelTopInset),
                   juce::Justification::topLeft, false);
    }

    void Keyboard::drawMarker(int midiNoteNumber, juce::Graphics& g, const juce::Rectangle<float>& area) const
    {
        const auto dot = markerDot(area);
        if (dot.isEmpty())
            return;

        const auto* colour = findMarker(midiNoteNumber);
        if (colour == nullptr)
            return;

        g.setColour(colour->withAlpha(1.0f));
        g.fillEllipse(dot);
    }

    juce::Rectangle<float> Keyboard::markerDot(const juce::Rectangle<float>& area) const
    {
        if (getOrientation() != horizontalKeyboard || area.getHeight() <= kMarkerDiameter + kMarkerBottomMargin)
            return {};

        const auto centre =
            juce::Point<float>(area.getCentreX(), area.getBottom() - kMarkerBottomMargin - kMarkerDiameter * 0.5f);

        return juce::Rectangle<float>(kMarkerDiameter, kMarkerDiameter).withCentre(centre);
    }

    const juce::Colour* Keyboard::findColor(int midiNoteNumber) const
    {
        const auto it = keyColours_.find(midiNoteNumber);
        if (it == keyColours_.end())
            return nullptr;

        return &it->second;
    }

    const juce::Colour* Keyboard::findMarker(int midiNoteNumber) const
    {
        const auto it = keyMarkers_.find(midiNoteNumber);
        if (it == keyMarkers_.end())
            return nullptr;

        return &it->second;
    }

}
