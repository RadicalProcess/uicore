#include "GlissonSlider/Slider.h"

#include <utility>
#include "Style.h"

#include "GlissonSlider/HorizontalKnob.h"
#include "GlissonSlider/VerticalKnob.h"

namespace rp::uicore::glisson
{

    Slider::Slider()
    : enabled_(true)
    , enabledRenderer_(knobs_)
    , disabledRenderer_(knobs_)
    , renderer_(&disabledRenderer_)
    {
    }

    void Slider::paint(juce::Graphics& g)
    {
        renderer_->paint(g, getLocalBounds().toFloat());
    }

    void Slider::mouseDown(const juce::MouseEvent& event)
    {
        for(auto& knob : knobs_)
        {
            if(knob->isClose(event.position))
            {
                knob->dragged_ = true;
                break;
            }
        }
    }

    void Slider::mouseDrag(const juce::MouseEvent& event)
    {
        auto itr = std::find_if(knobs_.begin(), knobs_.end(), [](const std::unique_ptr<Knob>& knob){
            return knob->dragged_;
        });
        if(itr == knobs_.end())
            return;

        (*itr)->setPoint(event.position);
        repaint();
    }

    void Slider::mouseUp(const juce::MouseEvent& event)
    {
        for (auto& knob : knobs_)
            knob->dragged_ = false;
    }

    void Slider::setStartLeft(float value)
    {
        knobs_[knobs_[0]->getValue() <= knobs_[1]->getValue() ? 0 : 1]->setValue(value);
    }

    void Slider::setStartRight(float value)
    {
        knobs_[knobs_[0]->getValue() <= knobs_[1]->getValue() ? 1 : 0]->setValue(value);
    }

    void Slider::setEndLeft(float value)
    {
        knobs_[knobs_[2]->getValue() <= knobs_[3]->getValue() ? 2 : 3]->setValue(value);
    }

    void Slider::setEndRight(float value)
    {
        knobs_[knobs_[2]->getValue() <= knobs_[3]->getValue() ? 3 : 2]->setValue(value);
    }

    void Slider::setStartMin(float value)
    {
        knobs_[knobs_[4]->getValue() <= knobs_[4]->getValue() ? 4 : 5]->setValue(value);
    }

    void Slider::setStartMax(float value)
    {
        knobs_[knobs_[4]->getValue() <= knobs_[5]->getValue() ? 5 : 4]->setValue(value);
    }

    void Slider::setEndMin(float value)
    {
        knobs_[knobs_[6]->getValue() <= knobs_[7]->getValue() ? 6 : 7]->setValue(value);
    }

    void Slider::setEndMax(float value)
    {
        knobs_[knobs_[7]->getValue() <= knobs_[6]->getValue() ? 7 : 6]->setValue(value);
    }

    void Slider::resized()
    {
        const auto localBounds = getLocalBounds();
        knobs_.clear();

        knobs_.emplace_back(std::make_unique<HorizontalKnob>(0.5f, localBounds.getWidth(), 5.0f));
        knobs_.emplace_back(std::make_unique<HorizontalKnob>(0.5f, localBounds.getWidth(), 5.0f));

        knobs_.emplace_back(std::make_unique<HorizontalKnob>(0.5f, localBounds.getWidth(), localBounds.getHeight() - 5.0f));
        knobs_.emplace_back(std::make_unique<HorizontalKnob>(0.5f, localBounds.getWidth(), localBounds.getHeight() - 5.0f));

        knobs_.emplace_back(std::make_unique<VerticalKnob>(0.5f, localBounds.getHeight(), 5.0f));
        knobs_.emplace_back(std::make_unique<VerticalKnob>(0.5f, localBounds.getHeight(), 5.0f));

        knobs_.emplace_back(std::make_unique<VerticalKnob>(0.5f, localBounds.getHeight(), localBounds.getWidth() - 5.0f));
        knobs_.emplace_back(std::make_unique<VerticalKnob>(0.5f, localBounds.getHeight(), localBounds.getWidth() - 5.0f));
    }
}
