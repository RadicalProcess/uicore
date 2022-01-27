#include "GlissonSlider/EnabledRenderer.h"
#include "Style.h"

namespace rp::uicore::glisson
{
    EnabledRenderer::EnabledRenderer(const std::vector<std::unique_ptr<Knob>>& knobs)
    : knobs_(knobs)
    {
    }

    void EnabledRenderer::paint(juce::Graphics& g, const juce::Rectangle<float>& bounds)
    {
        auto circleBounds = juce::Rectangle<float>(0.f, 0.f, 7.f, 7.f);

        g.setColour(styles::background);
        g.drawRect(3.f, 3.f, bounds.getWidth()-6.f, bounds.getHeight()-6.f, 3.f);
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

        drawLine(g, knobs_[top.first]->getPoint(), knobs_[bottom.first]->getPoint());
        drawLine(g, knobs_[top.second]->getPoint(), knobs_[bottom.second]->getPoint());
        drawLine(g, knobs_[left.first]->getPoint(), knobs_[right.first]->getPoint());
        drawLine(g, knobs_[left.second]->getPoint(), knobs_[right.second]->getPoint());
    }
}
