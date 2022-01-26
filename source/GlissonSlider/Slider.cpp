#include "GlissonSlider/Slider.h"

#include <utility>
#include "Style.h"

#include "GlissonSlider/HorizontalKnob.h"
#include "GlissonSlider/VerticalKnob.h"

namespace rp::uicore::glisson
{

    Slider::Slider()
    {
    }

    void Slider::paint(juce::Graphics& g)
    {
        const auto localBounds = getLocalBounds();
        auto circleBounds = juce::Rectangle<float>(0.f, 0.f, 7.f, 7.f);

        g.setColour(styles::background);
        g.drawRect(3.f, 3.f, localBounds.getWidth()-6.f, localBounds.getHeight()-6.f, 3.f);
        g.setColour(styles::highlight);

        for(auto& knob : knobs_)
        {
            circleBounds.setCentre(knob->getPoint());
            g.fillEllipse(circleBounds);
        }

        g.setColour(styles::foreground);
        for(auto i = 0; i < 4; ++i)
        {
            const auto pointA = knobs_[i*2]->getPoint();
            const auto pointB = knobs_[i*2+1]->getPoint();
            g.drawLine(pointA.getX(), pointA.getY(), pointB.getX(), pointB.getY(), 3.f);
        }

        g.setColour(styles::foreground);
        auto top = knobs_[0]->getValue() <= knobs_[1]->getValue() ? std::pair<size_t, size_t>(0, 1) : std::pair<size_t, size_t>(1, 0);
        auto bottom = knobs_[2]->getValue() <= knobs_[3]->getValue() ? std::pair<size_t, size_t>(2, 3) : std::pair<size_t, size_t>(3, 2);
        auto left = knobs_[4]->getValue() <= knobs_[5]->getValue() ? std::pair<size_t, size_t>(4, 5) : std::pair<size_t, size_t>(5, 4);
        auto right = knobs_[6]->getValue() <= knobs_[7]->getValue() ? std::pair<size_t, size_t>(6, 7) : std::pair<size_t, size_t>(7, 6);

        const auto drawLine = [&](const juce::Point<float>& from, const juce::Point<float>& to){
            g.drawLine(from.getX(), from.getY(), to.getX(), to.getY());
        };
        drawLine(knobs_[top.first]->getPoint(), knobs_[bottom.first]->getPoint());
        drawLine(knobs_[top.second]->getPoint(), knobs_[bottom.second]->getPoint());
        drawLine(knobs_[left.first]->getPoint(), knobs_[right.first]->getPoint());
        drawLine(knobs_[left.second]->getPoint(), knobs_[right.second]->getPoint());
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

        knobs_.emplace_back(std::make_unique<VerticalKnob>(0.5f, localBounds.getHeight(), localBounds.getHeight() - 5.0f));
        knobs_.emplace_back(std::make_unique<VerticalKnob>(0.5f, localBounds.getHeight(), localBounds.getHeight() - 5.0f));
    }
}
