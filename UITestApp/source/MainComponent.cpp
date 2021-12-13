#include "MainComponent.h"
#include <BinaryData.h>

namespace rp::uitest
{
    MainComponent::MainComponent()
    : label_("min length")
    , rSlider_("rslider", 1, "ms")
    , vSlider_("vslider")
    , sSlider_("sslider", 1, 5)
    , crSlider_("crslider")
    , hSlider_("hslider")
    , textField_()
    , logoImage_(juce::ImageCache::getFromMemory(BinaryData::Logo_png, BinaryData::Logo_pngSize))
    {
        setSize(800, 400);

        label_.setBounds(15, 255, 100, 20);
        addAndMakeVisible(label_);

        rSlider_.setBounds(10, 280, 80, 80);
        addAndMakeVisible(rSlider_);

        vSlider_.setBounds(100, 280, 15, 80);
        addAndMakeVisible(vSlider_);

        sSlider_.setBounds(130, 280, 80, 80);
        addAndMakeVisible(sSlider_);

        crSlider_.setBounds(220, 280, 80, 80);
        addAndMakeVisible(crSlider_);

        hSlider_.setBounds(310, 310, 80, 15);
        addAndMakeVisible(hSlider_);

        textField_.setBounds(390, 310, 80, 20);
        addAndMakeVisible(textField_);
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
