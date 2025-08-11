#include "ImageToggleButton.h"

namespace rp::uicore
{
    ImageToggleButton::ImageToggleButton( const juce::Image& imageOff, const juce::Image& imageOn)
    : juce::ToggleButton("")
    , imageOff_(imageOff)
    , imageOn_ (imageOn)
    {
        repaint();
    }

    void ImageToggleButton::paintButton(juce::Graphics& g, bool /*isMouseOverButton*/, bool /*isButtonDown*/)
    {
        const auto& img = getToggleState() ? imageOn_ : imageOff_;
        g.drawImageWithin(img, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::fillDestination);
    }

}
