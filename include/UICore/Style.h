#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore::styles
{
    const auto highlight = juce::Colour(juce::Colour(247, 174, 101));
    const auto background = juce::Colour(juce::Colour(5, 61, 87));
    const auto foreground = juce::Colour(juce::Colour(151, 188, 199));
    const auto text = juce::Colours::white;
    const auto playing = juce::Colour(juce::Colour(96, 209, 132));

    // Default plate, border and glyph colours of an IconButton, unselected and
    // selected. Every one of them can be overridden per button through the
    // button's ColourIds.
    const auto iconPlate = juce::Colour(0xff131822);
    const auto iconPlateSelected = juce::Colour(0xff182338);
    const auto iconBorder = juce::Colour(0xff2e3949);
    const auto iconBorderSelected = juce::Colour(0xff3a6ad8);
    const auto iconGlyph = juce::Colour(0xff75818f);
    const auto iconGlyphSelected = juce::Colour(0xffd2e6f8);

    const auto strokeType = juce::PathStrokeType( 3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
}
