#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore
{

    // Draws the small white number label that floats centred above a node
    // marker. TrajectoryView and ElevationView both label their nodes with
    // this, so a node carries the same one-based number in the same styling in
    // either view. centre is the marker's centre in pixels and radius its
    // radius; the label sits just above the marker and follows it as it moves.
    void drawNodeLabel(juce::Graphics& g, juce::Point<float> centre, float radius, int number);

    // Draws a round node marker centred on centre with the given radius, in the
    // shared style used by TrajectoryView, ElevationView and MotionView: a
    // filled highlight disc when highlighted (selected / dragged), otherwise a
    // hollow foreground ring over the background so the marker reads on top of
    // the curve. Callers overlay drawNodeLabel for the numbered views.
    void drawNodeMarker(juce::Graphics& g, juce::Point<float> centre, float radius, bool highlighted);

}
