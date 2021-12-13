#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Style.h"

namespace rp::uicore
{
    class TextFieldLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        TextFieldLookAndFeel()
        {
            setColour(juce::TextEditor::backgroundColourId, styles::background);
            setColour(juce::TextEditor::textColourId, styles::text);
            setColour(juce::TextEditor::highlightColourId, styles::highlight);
            setColour(juce::TextEditor::highlightedTextColourId, styles::background);
            setColour(juce::TextEditor::outlineColourId, styles::background);
            setColour(juce::TextEditor::focusedOutlineColourId, styles::background);
            setColour(juce::TextEditor::shadowColourId, styles::foreground);
        }
    };

    class TextField : public juce::TextEditor
    {
    public:
        TextField()
        {
            setLookAndFeel(&lf_);
        }

    private:
        TextFieldLookAndFeel lf_;
    };
}
