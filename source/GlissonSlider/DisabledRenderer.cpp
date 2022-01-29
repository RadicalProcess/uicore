#include "GlissonSlider/DisabledRenderer.h"
#include "GlissonSlider/KnobId.h"
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

        for(auto index : {TopA, TopB, LeftA, LeftB})
        {
            circleBounds.setCentre(knobs_[index]->getPoint());
            g.fillEllipse(circleBounds);
        }

        g.setColour(styles::foreground);
        for(auto knobId : {TopA, LeftA})
        {
            const auto pointA = knobs_[knobId]->getPoint();
            const auto pointB = knobs_[knobId+1]->getPoint();
            g.drawLine(pointA.getX(), pointA.getY(), pointB.getX(), pointB.getY(), 3.f);
        }

        g.setColour(styles::foreground);

        const auto width = bounds.getWidth() - 8.0f;
        const auto height = bounds.getHeight() - 8.0f;

        drawLine(g, knobs_[TopA]->getPoint(), knobs_[TopA]->getPoint().translated(0.0f, height));
        drawLine(g, knobs_[TopB]->getPoint(), knobs_[TopB]->getPoint().translated(0.0f, height));
        drawLine(g, knobs_[LeftA]->getPoint(), knobs_[LeftA]->getPoint().translated(width, 0.0f));
        drawLine(g, knobs_[LeftB]->getPoint(), knobs_[LeftB]->getPoint().translated(width, 0.0f));
    }
}
