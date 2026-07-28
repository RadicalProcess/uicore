#include "Icon.h"

namespace rp::uicore
{
    std::unique_ptr<juce::Drawable> loadWhiteIcon(const void* data, size_t size)
    {
        const auto svg = juce::String::createStringFromData(data, static_cast<int>(size))
                             .replace("currentColor", "#FFFFFF");

        const auto xml = juce::parseXML(svg);

        if (xml == nullptr)
        {
            return nullptr;
        }

        return juce::Drawable::createFromSVG(*xml);
    }

    void tintIcon(juce::Drawable* icon, juce::Colour& currentColour, juce::Colour colour)
    {
        if (icon == nullptr || currentColour == colour)
        {
            return;
        }

        icon->replaceColour(currentColour, colour);
        currentColour = colour;
    }
}
