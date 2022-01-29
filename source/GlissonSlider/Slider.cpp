#include "GlissonSlider/Slider.h"

#include <utility>
#include "Style.h"

#include "GlissonSlider/HorizontalKnob.h"
#include "GlissonSlider/VerticalKnob.h"

namespace rp::uicore::glisson
{
    Slider::Slider()
    : enabledRenderer_(knobs_)
    , disabledRenderer_(knobs_)
    , renderer_(&enabledRenderer_)
    {
        const auto width = 110.0f;
        const auto height = 80.0f;

        knobs_.emplace_back(std::make_unique<HorizontalKnob>(0.5f, width, 5.0f));
        knobs_.emplace_back(std::make_unique<HorizontalKnob>(0.5f, width, 5.0f));
        knobs_.emplace_back(std::make_unique<HorizontalKnob>(0.5f, width, height - 5.0f));
        knobs_.emplace_back(std::make_unique<HorizontalKnob>(0.5f, width, height - 5.0f));
        knobs_.emplace_back(std::make_unique<VerticalKnob>(0.5f, height, 5.0f));
        knobs_.emplace_back(std::make_unique<VerticalKnob>(0.5f, height, 5.0f));
        knobs_.emplace_back(std::make_unique<VerticalKnob>(0.5f, height, width - 5.0f));
        knobs_.emplace_back(std::make_unique<VerticalKnob>(0.5f, height, width - 5.0f));
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

        for(auto* listener : listeners_)
            listener->onSliderValueChanged(static_cast<KnobId>(itr - knobs_.begin()), (*itr)->getValue());

    }

    void Slider::mouseUp(const juce::MouseEvent& event)
    {
        for (auto& knob : knobs_)
            knob->dragged_ = false;
    }

    void Slider::set(KnobId knobId, float value)
    {
        knobs_[knobId]->setValue(value);
        repaint();
    }

    void Slider::setGlissonEnabled(bool enabled)
    {
        if(enabled)
            renderer_ = &enabledRenderer_;
        else
            renderer_ = &disabledRenderer_;
        repaint();
    }

    void Slider::addListener(Slider::Listener* listener)
    {
        listeners_.insert(listener);
    }

    void Slider::removeListener(Slider::Listener* listener)
    {
        listeners_.erase(listener);
    }

    void Slider::resized()
    {

    }
}
