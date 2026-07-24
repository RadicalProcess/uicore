#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <vector>

namespace rp::uicore
{
    // A Logic Pro-style swatch grid: 4 rows x 24 columns of preset colours the
    // user clicks to pick one. The colours are generated once (24 hues across the
    // spectrum, each in four descending brightness rows) and exposed through
    // palette(). Meant to be shown in a popup (e.g. a juce::CallOutBox); the
    // component sizes itself so the callout lays it out correctly.
    class ColorPalette : public juce::Component
    {
    public:
        static constexpr int kColumns = 24;
        static constexpr int kRows = 4;

        ColorPalette();

        // The 96 preset colours, laid out row-major (index = row * kColumns + col).
        static const std::vector<juce::Colour>& palette();

        // Highlight the swatch matching this colour, if the palette contains it.
        void setSelectedColour(juce::Colour colour);

        void paint(juce::Graphics& g) override;
        void mouseUp(const juce::MouseEvent& event) override;

        // Called with the picked colour when the user clicks a swatch.
        std::function<void(juce::Colour)> onColourPicked;

    private:
        // The swatch index under a point, or -1 when the point misses the grid.
        int indexAt(juce::Point<int> position) const;

        juce::Colour selectedColour_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColorPalette)
    };
}
