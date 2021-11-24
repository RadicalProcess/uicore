#include "MainComponent.h"

namespace rp::uitest
{
    MainComponent::MainComponent()
    : label_("min length")
    , slider_("slider")
    {
        setSize(600, 400);

        label_.setBounds(10, 10, 100, 50);
        addAndMakeVisible(label_);

        slider_.setBounds(10, 30, 100, 100);
        addAndMakeVisible(slider_);
    }

    void MainComponent::paint(juce::Graphics &g)
    {
        g.fillAll(juce::Colours::white);


    }

    void MainComponent::resized()
    {

    }

}