#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore::glisson
{
    class IRenderer
    {
    public:
        virtual ~IRenderer() = default;

        virtual void paint(juce::Graphics& g, const juce::Rectangle<float>& bounds) = 0;

    protected:
        void drawLine(juce::Graphics& g, const juce::Point<float>& from, const juce::Point<float>& to)
        {
            g.drawLine(from.getX(), from.getY(), to.getX(), to.getY());
        }
    };
}
