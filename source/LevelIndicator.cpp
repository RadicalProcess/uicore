#include "LevelIndicator.h"
#include "Style.h"

#include <algorithm>

namespace rp::uicore
{
    LevelIndicator::LevelIndicator(const std::string& name, Orientation orientation)
    : level_(0.0f)
    , orientation_(orientation)
    {
        setName(name);
    }

    LevelIndicator::~LevelIndicator()
    {
    }

    void LevelIndicator::setLevel(float level)
    {
        const auto clamped = juce::jlimit(0.0f, 1.0f, level);
        if (clamped == level_)
        {
            return;
        }

        level_ = clamped;
        repaint();
    }

    void LevelIndicator::paint(juce::Graphics& g)
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto corner = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;

        g.setColour(styles::background);
        g.fillRoundedRectangle(bounds, corner);

        if (level_ <= 0.0f)
        {
            return;
        }

        g.setColour(styles::highlight);

        if (orientation_ == Orientation::Horizontal)
        {
            const auto filledWidth = bounds.getWidth() * level_;
            g.fillRoundedRectangle(bounds.withWidth(filledWidth), corner);
            return;
        }

        const auto filledHeight = bounds.getHeight() * level_;
        g.fillRoundedRectangle(bounds.withTop(bounds.getBottom() - filledHeight), corner);
    }
}
