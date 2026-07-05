#include "IconButton.h"
#include "Style.h"
#include <BinaryData.h>

namespace rp::uicore
{
    namespace
    {
        // The bundled SVG icons use stroke="currentColor", which JUCE resolves
        // to transparent black (invisible). Swap it for white before parsing so
        // the glyph is drawn in the foreground colour.
        std::unique_ptr<juce::Drawable> loadWhiteIcon(const void* data, size_t size)
        {
            const auto svg = juce::String::createStringFromData(data, static_cast<int>(size))
                                 .replace("currentColor", "#FFFFFF");

            const auto xml = juce::parseXML(svg);

            if (xml == nullptr)
            {
                return nullptr;
            }

            return juce::Drawable::createFromSVG(*xml);
        }
    }

    IconButton::IconButton(const void* svgData, size_t svgDataSize)
    : juce::Button("")
    , iconOff_(loadWhiteIcon(svgData, svgDataSize))
    {
    }

    IconButton::IconButton(const void* svgOffData, size_t svgOffDataSize,
                           const void* svgOnData, size_t svgOnDataSize)
    : juce::Button("")
    , iconOff_(loadWhiteIcon(svgOffData, svgOffDataSize))
    , iconOn_ (loadWhiteIcon(svgOnData, svgOnDataSize))
    {
        setClickingTogglesState(true);
    }

    void IconButton::setShape(Shape shape)
    {
        shape_ = shape;
        repaint();
    }

    void IconButton::paintButton(juce::Graphics& g, bool /*isMouseOverButton*/, bool isButtonDown)
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto active = isButtonDown || getToggleState();

        g.setColour(active ? styles::highlight : styles::background);

        if (shape_ == Shape::Circle)
        {
            g.fillEllipse(bounds);
        }
        else
        {
            const auto corner = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.25f;
            g.fillRoundedRectangle(bounds, corner);
        }

        const auto* icon = (getToggleState() && iconOn_ != nullptr) ? iconOn_.get()
                                                                    : iconOff_.get();

        if (icon == nullptr)
        {
            return;
        }

        const auto iconBounds = bounds.reduced(bounds.getWidth() * 0.28f,
                                               bounds.getHeight() * 0.28f);
        icon->drawWithin(g, iconBounds, juce::RectanglePlacement::centred, 1.0f);
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
}
