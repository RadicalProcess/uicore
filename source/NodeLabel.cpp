#include "UICore/NodeLabel.h"
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
        g.setFont(labelHeight_);
        g.drawText(juce::String(number), area, juce::Justification::centred, false);
    }

}
