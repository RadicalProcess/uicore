#include "GlissonSlider/EnabledRenderer.h"
#include "Style.h"
#include "GlissonSlider/KnobId.h"

namespace rp::uicore::glisson
{
    namespace
    {
        Knob& getMin(Knob& knobA, Knob& knobB)
        {
            return knobA.getValue() <= knobB.getValue() ? knobA : knobB;
        }

        Knob& getMax(Knob& knobA, Knob& knobB)
        {
            return knobA.getValue() >= knobB.getValue() ? knobA : knobB;
        }
    }

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

        drawLine(g, getMin(*knobs_[TopA], *knobs_[TopB]).getPoint(), getMin(*knobs_[BottomA], *knobs_[BottomB]).getPoint());
        drawLine(g, getMax(*knobs_[TopA], *knobs_[TopB]).getPoint(), getMax(*knobs_[BottomA], *knobs_[BottomB]).getPoint());
        drawLine(g, getMin(*knobs_[LeftA], *knobs_[LeftB]).getPoint(), getMin(*knobs_[RightA], *knobs_[RightB]).getPoint());
        drawLine(g, getMax(*knobs_[LeftA], *knobs_[LeftB]).getPoint(), getMax(*knobs_[RightA], *knobs_[RightB]).getPoint());
    }
}
