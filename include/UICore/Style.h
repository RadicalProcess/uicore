#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore::styles
{
    const auto highlight = juce::Colour(juce::Colour(247, 174, 101));
    const auto background = juce::Colour(juce::Colour(5, 61, 87));
    const auto foreground = juce::Colour(juce::Colour(151, 188, 199));
    const auto text = juce::Colours::white;

    // Fill for the editor canvases (trajectory / elevation / motion / waveform
    // / gesture) and the combo box: a very dark grey so the coloured content
    // reads clearly without the flatness of pure black.
    const auto canvasBackground = juce::Colour(30, 30, 30);

    // Secondary tones, derived so they harmonise with the palette above.
    const auto frame = foreground.withAlpha(0.4f);  // frames, zero/reference/grid lines, handle stems
    const auto mutedText = text.withAlpha(0.5f);    // placeholder / secondary text

    // Stroke widths, so the same element reads at the same weight in every view.
    const auto playheadStroke = 2.5f;   // playhead crossing lines
    const auto curveStroke = 2.0f;      // trajectory curve / elevation graph
    const auto guideStroke = 1.5f;      // frames, segments, handle stems, node rings, fade lines
    const auto hairlineStroke = 1.0f;   // selection edges, 1px rects, combo arrow
    const auto heavyStroke = 3.0f;      // GlissonSlider value bars

    // Typography + shape.
    const auto labelFontHeight = 12.0f; // numbered node / reference labels
    const auto cornerRadius = 5.0f;     // rounded panels

    const auto strokeType = juce::PathStrokeType( 3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
}
