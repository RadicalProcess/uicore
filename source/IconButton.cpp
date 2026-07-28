#include "IconButton.h"
#include "Icon.h"
#include "Style.h"
#include <BinaryData.h>

namespace rp::uicore
{
    namespace
    {
        // Thickness of the plate's border, in pixels.
        constexpr float kBorderThickness = 1.0f;

        // Corner radius of a rounded square plate, as a fraction of its size.
        constexpr float kCornerFactor = 0.2f;

        // Margin around the glyph, as a fraction of the plate's size.
        constexpr float kGlyphMargin = 0.22f;

        // Opacity of a disabled button.
        constexpr float kDisabledOpacity = 0.4f;
    }

    IconButton::IconButton(const void* svgData, size_t svgDataSize)
    : juce::Button("")
    , iconOff_(loadWhiteIcon(svgData, svgDataSize))
    , iconOffColour_(juce::Colours::white)
    , iconOnColour_(juce::Colours::white)
    {
        applyDefaultColours();
    }

    IconButton::IconButton(const void* svgOffData, size_t svgOffDataSize,
                           const void* svgOnData, size_t svgOnDataSize)
    : juce::Button("")
    , iconOff_(loadWhiteIcon(svgOffData, svgOffDataSize))
    , iconOn_ (loadWhiteIcon(svgOnData, svgOnDataSize))
    , iconOffColour_(juce::Colours::white)
    , iconOnColour_(juce::Colours::white)
    {
        setClickingTogglesState(true);
        applyDefaultColours();
    }

    void IconButton::setShape(Shape shape)
    {
        shape_ = shape;
        repaint();
    }

    void IconButton::applyDefaultColours()
    {
        setColour(plateColourId, styles::iconPlate);
        setColour(plateSelectedColourId, styles::iconPlateSelected);
        setColour(borderColourId, styles::iconBorder);
        setColour(borderSelectedColourId, styles::iconBorderSelected);
        setColour(glyphColourId, styles::iconGlyph);
        setColour(glyphSelectedColourId, styles::iconGlyphSelected);
    }

    void IconButton::paintButton(juce::Graphics& g, bool /*isMouseOverButton*/, bool isButtonDown)
    {
        const auto bounds = getLocalBounds().toFloat().reduced(kBorderThickness * 0.5f);
        const auto selected = isButtonDown || getToggleState();
        const auto opacity = isEnabled() ? 1.0f : kDisabledOpacity;
        const auto plate = findColour(selected ? plateSelectedColourId : plateColourId);
        const auto border = findColour(selected ? borderSelectedColourId : borderColourId);
        const auto corner = juce::jmin(bounds.getWidth(), bounds.getHeight()) * kCornerFactor;

        g.setColour(plate.withMultipliedAlpha(opacity));

        if (shape_ == Shape::Circle)
        {
            g.fillEllipse(bounds);
        }
        else
        {
            g.fillRoundedRectangle(bounds, corner);
        }

        g.setColour(border.withMultipliedAlpha(opacity));

        if (shape_ == Shape::Circle)
        {
            g.drawEllipse(bounds, kBorderThickness);
        }
        else
        {
            g.drawRoundedRectangle(bounds, corner, kBorderThickness);
        }

        const auto showsOnIcon = getToggleState() && iconOn_ != nullptr;
        auto* icon = showsOnIcon ? iconOn_.get() : iconOff_.get();

        if (icon == nullptr)
        {
            return;
        }

        tintIcon(icon, showsOnIcon ? iconOnColour_ : iconOffColour_,
                 findColour(selected ? glyphSelectedColourId : glyphColourId));

        const auto iconBounds = bounds.reduced(bounds.getWidth() * kGlyphMargin,
                                               bounds.getHeight() * kGlyphMargin);
        icon->drawWithin(g, iconBounds, juce::RectanglePlacement::centred, opacity);
    }

    void IconButton::colourChanged()
    {
        repaint();
    }

    PlayPauseButton::PlayPauseButton()
    : IconButton(BinaryData::play_svg, BinaryData::play_svgSize,
                 BinaryData::pause_svg, BinaryData::pause_svgSize)
    {
    }

    StopButton::StopButton()
    : IconButton(BinaryData::stop_svg, BinaryData::stop_svgSize)
    {
    }

    TrashButton::TrashButton()
    : IconButton(BinaryData::trash_svg, BinaryData::trash_svgSize)
    {
        setShape(Shape::RoundedSquare);
    }

    CopyButton::CopyButton()
    : IconButton(BinaryData::copy_svg, BinaryData::copy_svgSize)
    {
        setShape(Shape::RoundedSquare);
    }

    RoomButton::RoomButton()
    : IconButton(BinaryData::room_svg, BinaryData::room_svgSize)
    {
        setShape(Shape::RoundedSquare);
    }

    EditButton::EditButton()
    : IconButton(BinaryData::pencil_svg, BinaryData::pencil_svgSize)
    {
        setShape(Shape::RoundedSquare);
        setClickingTogglesState(true);
    }

    EyeButton::EyeButton()
    : IconButton(BinaryData::eye_svg, BinaryData::eye_svgSize)
    {
        setShape(Shape::RoundedSquare);
        setClickingTogglesState(true);
    }

    PlusButton::PlusButton()
    : IconButton(BinaryData::plus_svg, BinaryData::plus_svgSize)
    {
    }

    MinusButton::MinusButton()
    : IconButton(BinaryData::minus_svg, BinaryData::minus_svgSize)
    {
    }

    SettingsButton::SettingsButton()
    : IconButton(BinaryData::settings_svg, BinaryData::settings_svgSize)
    {
        setShape(Shape::RoundedSquare);
    }

    CurveButton::CurveButton()
    : IconButton(BinaryData::chartline_svg, BinaryData::chartline_svgSize)
    {
        setShape(Shape::RoundedSquare);
    }

    TrimButton::TrimButton()
    : IconButton(BinaryData::scissors_svg, BinaryData::scissors_svgSize)
    {
        setShape(Shape::RoundedSquare);
    }

    WaveformButton::WaveformButton()
    : IconButton(BinaryData::wave_svg, BinaryData::wave_svgSize)
    {
        setShape(Shape::RoundedSquare);
    }

    WidthButton::WidthButton()
    : IconButton(BinaryData::width_svg, BinaryData::width_svgSize)
    {
        setShape(Shape::RoundedSquare);
    }
}
