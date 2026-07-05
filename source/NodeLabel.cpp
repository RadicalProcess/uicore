#include "UICore/NodeLabel.h"
#include "UICore/Font.h"
#include "UICore/Style.h"

namespace rp::uicore
{

    namespace
    {
        // Height (in pixels) of the label text and the box it is centred in,
        // the box width, and the gap kept between the box and the marker's top.
        const auto labelHeight_ = 12.0f;
        const auto labelWidth_ = 24.0f;
        const auto labelGap_ = 2.0f;
    }

    void drawNodeLabel(juce::Graphics& g, juce::Point<float> centre, float radius, int number)
    {
        const auto area = juce::Rectangle<float>(centre.x - labelWidth_ * 0.5f,
                                                 centre.y - radius - labelGap_ - labelHeight_,
                                                 labelWidth_,
                                                 labelHeight_);

        g.setColour(styles::text);
        g.setFont(getRobotoCondensed().withHeight(labelHeight_));
        g.drawText(juce::String(number), area, juce::Justification::centred, false);
    }

    void drawNodeMarker(juce::Graphics& g, juce::Point<float> centre, float radius, bool highlighted)
    {
        const auto bounds = juce::Rectangle<float>(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

        if (highlighted)
        {
            g.setColour(styles::highlight);
            g.fillEllipse(bounds);
            return;
        }

        g.setColour(styles::canvasBackground);
        g.fillEllipse(bounds);
        g.setColour(styles::foreground);
        g.drawEllipse(bounds, styles::guideStroke);
    }

}
