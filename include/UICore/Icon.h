#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <cstddef>
#include <memory>

namespace rp::uicore
{
    // Parses one of the bundled SVG icons into a drawable. The icons stroke with
    // "currentColor", which JUCE resolves to transparent black (invisible), so
    // the colour is swapped for white before parsing and the caller recolours
    // the white glyph with tintIcon. Returns nullptr when the data is not SVG.
    std::unique_ptr<juce::Drawable> loadWhiteIcon(const void* data, size_t size);

    // Recolours a glyph loaded by loadWhiteIcon in place. currentColour is the
    // colour the glyph carries now and is updated to colour, so the drawable is
    // only walked when the colour actually changes.
    void tintIcon(juce::Drawable* icon, juce::Colour& currentColour, juce::Colour colour);
}
