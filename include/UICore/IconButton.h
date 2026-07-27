#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore
{
    // A button that renders an SVG glyph on an outlined plate: the plate is
    // filled, bordered and the glyph tinted, all six colours coming from the
    // button's ColourIds so a host can restyle a single button or all of them.
    // A pressed or toggled button uses the selected colours.
    //
    // Construct with a single icon for a momentary button, or with an off/on
    // pair for a toggle button whose glyph swaps with the toggle state. Both
    // forms share the same styling so a toggle and a momentary button placed
    // side by side look identical.
    class IconButton : public juce::Button
    {
    public:
        // The shape of the plate the glyph sits on.
        enum class Shape
        {
            Circle,
            RoundedSquare
        };

        enum ColourIds
        {
            plateColourId = 0x2001000,
            plateSelectedColourId = 0x2001001,
            borderColourId = 0x2001002,
            borderSelectedColourId = 0x2001003,
            glyphColourId = 0x2001004,
            glyphSelectedColourId = 0x2001005
        };

        // Momentary button showing a single icon.
        IconButton(const void* svgData, size_t svgDataSize);

        // Toggle button showing svgOff when untoggled and svgOn when toggled.
        IconButton(const void* svgOffData, size_t svgOffDataSize,
                   const void* svgOnData, size_t svgOnDataSize);

        ~IconButton() override = default;

        void setShape(Shape shape);

    private:
        void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;

        void colourChanged() override;

        void applyDefaultColours();

        std::unique_ptr<juce::Drawable> iconOff_;
        std::unique_ptr<juce::Drawable> iconOn_;
        juce::Colour iconOffColour_;
        juce::Colour iconOnColour_;
        Shape shape_ = Shape::Circle;
    };

    // Concrete buttons with their icon, shape and momentary/toggle behaviour
    // baked in. Each is drop-in: default-construct, position and add it.

    // Circular toggle whose glyph swaps between play and pause.
    class PlayPauseButton : public IconButton
    {
    public:
        PlayPauseButton();
    };

    // Circular momentary button showing the stop glyph.
    class StopButton : public IconButton
    {
    public:
        StopButton();
    };

    // Rounded-square momentary button showing the trash glyph.
    class TrashButton : public IconButton
    {
    public:
        TrashButton();
    };

    // Rounded-square momentary button showing the copy glyph.
    class CopyButton : public IconButton
    {
    public:
        CopyButton();
    };

    // Rounded-square momentary button showing the room glyph.
    class RoomButton : public IconButton
    {
    public:
        RoomButton();
    };

    // Rounded-square toggle showing the edit glyph; highlights while active.
    class EditButton : public IconButton
    {
    public:
        EditButton();
    };

    // Rounded-square toggle showing the eye glyph; highlights while active.
    class EyeButton : public IconButton
    {
    public:
        EyeButton();
    };

    // Circular momentary button showing the plus glyph.
    class PlusButton : public IconButton
    {
    public:
        PlusButton();
    };

    // Circular momentary button showing the minus glyph.
    class MinusButton : public IconButton
    {
    public:
        MinusButton();
    };

    // Rounded-square momentary button showing the settings glyph.
    class SettingsButton : public IconButton
    {
    public:
        SettingsButton();
    };

    // Rounded-square momentary button showing the chart-line glyph.
    class CurveButton : public IconButton
    {
    public:
        CurveButton();
    };

    // Rounded-square momentary button showing the scissors glyph.
    class TrimButton : public IconButton
    {
    public:
        TrimButton();
    };

    // Rounded-square momentary button showing the wave glyph.
    class WaveformButton : public IconButton
    {
    public:
        WaveformButton();
    };

    // Rounded-square momentary button showing the width glyph.
    class WidthButton : public IconButton
    {
    public:
        WidthButton();
    };
}
