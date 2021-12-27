#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore::styles
{
    const auto highlight = juce::Colour(juce::Colour(247, 174, 101));
    const auto background = juce::Colour(juce::Colour(5, 61, 87));
    const auto foreground = juce::Colour(juce::Colour(151, 188, 199));
    const auto text = juce::Colours::white;

    const auto strokeType = juce::PathStrokeType( 3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
}
