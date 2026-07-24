#include "ColorPalette.h"

#include <array>
#include <utility>

namespace rp::uicore
{
    namespace
    {
        constexpr int kCellSize = 22;
        constexpr int kPadding = 8;
        constexpr float kSwatchCorner = 3.0f;
        constexpr float kSwatchGap = 2.0f;

        // Descending brightness/saturation per row, giving the Logic Pro look:
        // vivid on top, progressively darker underneath.
        constexpr std::array<std::pair<float, float>, ColorPalette::kRows> kRowShades{
            {{0.80f, 1.00f}, {0.90f, 0.80f}, {0.95f, 0.60f}, {0.95f, 0.42f}}};

        int gridExtent(int cells)
        {
            return cells * kCellSize + 2 * kPadding;
        }
    }

    ColorPalette::ColorPalette()
    {
        setSize(gridExtent(kColumns), gridExtent(kRows));
    }

    const std::vector<juce::Colour>& ColorPalette::palette()
    {
        static const std::vector<juce::Colour> colours = []
        {
            std::vector<juce::Colour> generated;
            generated.reserve(static_cast<size_t>(kRows * kColumns));

            for (int row = 0; row < kRows; ++row)
            {
                const auto shade = kRowShades[static_cast<size_t>(row)];
                for (int column = 0; column < kColumns; ++column)
                {
                    const auto hue = static_cast<float>(column) / static_cast<float>(kColumns);
                    generated.push_back(juce::Colour::fromHSV(hue, shade.first, shade.second, 1.0f));
                }
            }

            return generated;
        }();

        return colours;
    }

    void ColorPalette::setSelectedColour(juce::Colour colour)
    {
        if (selectedColour_ == colour)
            return;

        selectedColour_ = colour;
        repaint();
    }

    void ColorPalette::paint(juce::Graphics& g)
    {
        g.fillAll(juce::Colour(20, 20, 20));

        const auto& colours = palette();
        for (int row = 0; row < kRows; ++row)
        {
            for (int column = 0; column < kColumns; ++column)
            {
                const auto index = static_cast<size_t>(row * kColumns + column);
                const juce::Rectangle<float> cell(static_cast<float>(kPadding + column * kCellSize),
                                                  static_cast<float>(kPadding + row * kCellSize),
                                                  static_cast<float>(kCellSize), static_cast<float>(kCellSize));
                const auto swatch = cell.reduced(kSwatchGap);

                g.setColour(colours[index]);
                g.fillRoundedRectangle(swatch, kSwatchCorner);

                if (colours[index] == selectedColour_)
                {
                    g.setColour(juce::Colours::white);
                    g.drawRoundedRectangle(swatch, kSwatchCorner, 2.0f);
                }
            }
        }
    }

    int ColorPalette::indexAt(juce::Point<int> position) const
    {
        const auto x = position.getX() - kPadding;
        const auto y = position.getY() - kPadding;
        if (x < 0 || y < 0)
            return -1;

        const auto column = x / kCellSize;
        const auto row = y / kCellSize;
        if (column >= kColumns || row >= kRows)
            return -1;

        return row * kColumns + column;
    }

    void ColorPalette::mouseUp(const juce::MouseEvent& event)
    {
        const auto index = indexAt(event.getPosition());
        if (index < 0)
            return;

        if (onColourPicked)
            onColourPicked(palette()[static_cast<size_t>(index)]);
    }
}
