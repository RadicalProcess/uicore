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
            setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
            setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
            setColour(juce::TextEditor::shadowColourId, styles::foreground);
        }

    private:
        void fillTextEditorBackground (juce::Graphics& g, int /*width*/, int /*height*/, juce::TextEditor& textEditor) override
        {
            g.setColour(textEditor.findColour (juce::TextEditor::backgroundColourId));
            g.fillRoundedRectangle(textEditor.getLocalBounds().toFloat(), 10.0f);
        }
    };

    class TextField : public juce::TextEditor
    {
    public:
        TextField()
        {
            setFont(getRobotoCondensed());
            setLookAndFeel(&lf_);
        }

        ~TextField() override
        {
            setLookAndFeel(nullptr);
        }

    private:
        TextFieldLookAndFeel lf_;
    };
}
