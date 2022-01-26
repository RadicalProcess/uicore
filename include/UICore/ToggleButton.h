#pragma once
#include "Style.h"

namespace rp::uicore
{
    class ToggleButton : public juce::ToggleButton
    {
    public:
        explicit ToggleButton(const std::string& name)
        : juce::ToggleButton(name)
        {
        }

        ~ToggleButton() override
        {
        }

    private:

        void paintButton(juce::Graphics& g, bool , bool ) override
        {

            g.setColour(getToggleState() ? styles::highlight : styles::background);
            g.fillEllipse(getLocalBounds().toFloat());
        }
    };
}
