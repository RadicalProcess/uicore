#include "MainComponent.h"
#include <BinaryData.h>

namespace rp::uitest
{
    MainComponent::MainComponent()
    : label_("min length")
    , slider_("slider")
    , logoImage_(juce::ImageCache::getFromMemory(BinaryData::Logo_png, BinaryData::Logo_pngSize))
    {
        setSize(800, 400);

        label_.setBounds(15, 255, 100, 20);
        addAndMakeVisible(label_);

        slider_.setBounds(10, 280, 80, 80);
        addAndMakeVisible(slider_);
    }

    void MainComponent::paint(juce::Graphics &g)
    {
        g.fillAll(juce::Colours::black);

        g.drawImageWithin(logoImage_,530, 5, 265, 40, juce::RectanglePlacement::Flags::stretchToFit);
    }

    void MainComponent::resized()
    {

    }

}
