#include "ProgressBar.h"
#include "Style.h"

namespace rp::uicore
{
    ProgressBar::ProgressBar(const std::string& name)
    : progress_(0.0)
    {
        setName(name);
    }

    ProgressBar::~ProgressBar()
    {
    }

    void ProgressBar::setProgress(double progress)
    {
        const auto clamped = juce::jlimit(0.0, 1.0, progress);
        if (clamped == progress_)
        {
            return;
        }

        progress_ = clamped;
        repaint();
    }

    double ProgressBar::getProgress() const
    {
        return progress_;
    }

    void ProgressBar::paint(juce::Graphics& g)
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto corner = bounds.getHeight() * 0.5f;

        g.setColour(styles::background);
        g.fillRoundedRectangle(bounds, corner);

        if (progress_ <= 0.0)
        {
            return;
        }

        const auto filledWidth = static_cast<float>(bounds.getWidth() * progress_);
        g.setColour(styles::highlight);
        g.fillRoundedRectangle(bounds.withWidth(filledWidth), corner);
    }
}
