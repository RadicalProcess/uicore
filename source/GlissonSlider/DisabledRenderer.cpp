#include "GlissonSlider/DisabledRenderer.h"
#include "Style.h"

namespace rp::uicore::glisson
{
    DisabledRenderer::DisabledRenderer(const std::vector<std::unique_ptr<Knob>>& knobs)
    : knobs_(knobs)
    {

    }

    void DisabledRenderer::paint(juce::Graphics& g, const juce::Rectangle<float>& bounds)
    {
        auto circleBounds = juce::Rectangle<float>(0.f, 0.f, 7.f, 7.f);

        g.setColour(styles::background);
        g.drawRect(3.f, 3.f, bounds.getWidth()-6.f, bounds.getHeight()-6.f, 3.f);
        g.setColour(styles::highlight);

        for(auto index : {0, 1, 4, 5})
        {
            circleBounds.setCentre(knobs_[index]->getPoint());
            g.fillEllipse(circleBounds);
        }

        g.setColour(styles::foreground);
        for(auto i : {0, 2})
        {
            const auto pointA = knobs_[i*2]->getPoint();
            const auto pointB = knobs_[i*2+1]->getPoint();
            g.drawLine(pointA.getX(), pointA.getY(), pointB.getX(), pointB.getY(), 3.f);
        }

        g.setColour(styles::foreground);
        auto top = knobs_[0]->getValue() <= knobs_[1]->getValue() ? std::pair<size_t, size_t>(0, 1) : std::pair<size_t, size_t>(1, 0);
        auto left = knobs_[4]->getValue() <= knobs_[5]->getValue() ? std::pair<size_t, size_t>(4, 5) : std::pair<size_t, size_t>(5, 4);

        const auto width = bounds.getWidth() - 8.0f;
        const auto height = bounds.getHeight() - 8.0f;

        drawLine(g, knobs_[top.first]->getPoint(), knobs_[top.first]->getPoint().translated(0.0f, height));
        drawLine(g, knobs_[top.second]->getPoint(), knobs_[top.second]->getPoint().translated(0.0f, height));
        drawLine(g, knobs_[left.first]->getPoint(), knobs_[left.first]->getPoint().translated(width, 0.0f));
        drawLine(g, knobs_[left.second]->getPoint(), knobs_[left.second]->getPoint().translated(width, 0.0f));
    }
}
