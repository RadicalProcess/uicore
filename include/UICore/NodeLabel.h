#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore
{

    // Draws the small white number label that floats centred above a node
    // marker. TrajectoryView labels both its trajectory anchors and its
    // elevation nodes with this, so a node carries the same one-based number in
    // the same styling in either area. centre is the marker's centre in pixels
    // and radius its radius; the label sits just above the marker and follows it
    // as it moves.
    void drawNodeLabel(juce::Graphics& g, juce::Point<float> centre, float radius, int number);

}
