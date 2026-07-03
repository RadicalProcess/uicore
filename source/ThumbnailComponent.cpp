#include "ThumbnailComponent.h"

namespace rp::uicore
{
    ThumbnailComponent::ThumbnailComponent()
    {
    }

    void ThumbnailComponent::paint(juce::Graphics &g)
    {
        g.fillAll(juce::Colours::grey);
    }

}
