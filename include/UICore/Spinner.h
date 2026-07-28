#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>
#include <string>

namespace rp::uicore
{
    // An indeterminate busy indicator: the loader glyph turning at a constant
    // rate. Where ProgressBar shows how far a task has come, a Spinner only says
    // that one is still running, so it fits a job whose remaining time is not
    // known and a spot too narrow for a word. The glyph is drawn in
    // glyphColourId so a host can tint it to match whatever it sits in. The
    // animation runs only while the spinner is visible.
    class Spinner : public juce::Component, private juce::Timer
    {
    public:
        enum ColourIds
        {
            glyphColourId = 0x2006000
        };

        explicit Spinner(const std::string& name);

        ~Spinner() override;

    private:
        void paint(juce::Graphics& g) override;

        void visibilityChanged() override;

        void colourChanged() override;

        void timerCallback() override;

        std::unique_ptr<juce::Drawable> icon_;

        // The colour the glyph carries now, so it is only recoloured when the
        // host asks for a different one.
        juce::Colour iconColour_;

        // How far the glyph is currently turned, in radians.
        float angle_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Spinner)
    };
}
