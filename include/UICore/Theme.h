#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "LevelIndicator.h"
#include "Waveform.h"
#include "WaveformScrollBar.h"

namespace rp::uicore::theme
{
    // The dark theme shared by the Radical Process plugins. A plugin keeps only
    // its own layout constants and colours; everything that makes two plugins
    // look like one family lives here so it is defined once.
    extern const juce::Colour appBackground;
    extern const juce::Colour headerBar;
    extern const juce::Colour panelFill;
    extern const juce::Colour panelBorder;
    extern const juce::Colour insetWell;
    extern const juce::Colour selectedFill;
    extern const juce::Colour selectedStroke;
    extern const juce::Colour accent;
    extern const juce::Colour accentBright;
    extern const juce::Colour waveformTrace;
    extern const juce::Colour textPrimary;
    extern const juce::Colour textSecondary;
    extern const juce::Colour textHint;
    extern const juce::Colour iconFrame;
    extern const juce::Colour iconStroke;
    extern const juce::Colour viewFrame;
    extern const juce::Colour curve;
    extern const juce::Colour guide;
    extern const juce::Colour controlWell;
    extern const juce::Colour overlayDim;

    // Margin kept around the body panels, and the gap between two of them.
    extern const int bodyMargin;
    extern const int panelGap;

    // Roboto Condensed at the given height.
    juce::Font makeFont(float height);

    // Fills the bounds as a themed panel: rounded rect with a 1 px border.
    void drawPanel(juce::Graphics& g, juce::Rectangle<int> bounds);

    // Fills the bounds as the header strip of a panel: the header colour with
    // the panel's rounded corners at the top, square at the bottom, and a border
    // line separating it from the panel body below.
    void drawPanelHeader(juce::Graphics& g, juce::Rectangle<int> bounds);

    // Fills the bounds as a themed drawing surface: the same rounded rect and
    // border as a panel, but sunk to the darker well colour the editor views
    // draw their curves on.
    void drawViewPanel(juce::Graphics& g, juce::Rectangle<int> bounds);

    // Styles a label as a panel section title: letter-spaced secondary text.
    // The title text itself is expected to be ALL-CAPS.
    void styleSectionTitle(juce::Label& label);

    // Gives a rotary slider the theme's dial colours: sunken plate, icon-frame
    // rim, and a bright pointer and reading.
    void styleKnob(juce::Slider& slider);

    // Gives a level meter the theme's readout colours: sunken track and a
    // slightly softened bright accent for the level.
    void styleMeter(LevelIndicator& meter);

    // Gives a waveform the theme's colours: sunken background, panel border,
    // muted trace and bright accent marks.
    void styleWaveform(Waveform& waveform);

    // Gives a waveform scroll bar the theme's colours, matching styleWaveform.
    void styleWaveformScrollBar(WaveformScrollBar& scrollBar);
}
