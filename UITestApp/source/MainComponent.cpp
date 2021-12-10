#include "MainComponent.h"
#include <BinaryData.h>

namespace rp::uitest
{
    MainComponent::MainComponent()
    : label_("min length")
    , rSlider_("rslider")
    , vSlider_("vslider")
    , logoImage_(juce::ImageCache::getFromMemory(BinaryData::Logo_png, BinaryData::Logo_pngSize))
    {
        setSize(800, 400);

        label_.setBounds(15, 255, 100, 20);
        addAndMakeVisible(label_);

        rSlider_.setBounds(10, 280, 80, 80);
        addAndMakeVisible(rSlider_);

        vSlider_.setBounds(100, 280, 15, 80);
        addAndMakeVisible(vSlider_);
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
