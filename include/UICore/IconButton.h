#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore
{
    // A round button that renders a white SVG glyph on a coloured disc.
    //
    // Construct with a single icon for a momentary button, or with an off/on
    // pair for a toggle button whose glyph swaps with the toggle state. Both
    // forms share the same styling so a toggle and a momentary button placed
    // side by side look identical.
    class IconButton : public juce::Button
    {
    public:
        // The shape of the coloured disc/plate the glyph sits on.
        enum class Shape
        {
            Circle,
            RoundedSquare
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

        std::unique_ptr<juce::Drawable> iconOff_;
        std::unique_ptr<juce::Drawable> iconOn_;
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
