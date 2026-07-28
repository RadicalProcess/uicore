#include "Spinner.h"
#include "Icon.h"
#include "Style.h"

#include <BinaryData.h>

namespace rp::uicore
{
    namespace
    {
        // Rate the rotation is redrawn at, and how long one full turn takes.
        constexpr int kFramesPerSecond = 30;
        constexpr float kSecondsPerTurn = 1.2f;

        constexpr float kRadiansPerFrame =
            juce::MathConstants<float>::twoPi / (static_cast<float>(kFramesPerSecond) * kSecondsPerTurn);

        // Margin around the glyph, as a fraction of the spinner's size.
        constexpr float kGlyphMargin = 0.1f;
    }

    Spinner::Spinner(const std::string& name)
    : icon_(loadWhiteIcon(BinaryData::loader_svg, BinaryData::loader_svgSize))
    , iconColour_(juce::Colours::white)
    , angle_(0.0f)
    {
        setName(name);
        setColour(glyphColourId, styles::highlight);
    }

    Spinner::~Spinner()
    {
    }

    void Spinner::paint(juce::Graphics& g)
    {
        if (icon_ == nullptr)
        {
            return;
        }

        tintIcon(icon_.get(), iconColour_, findColour(glyphColourId));

        const auto bounds = getLocalBounds().toFloat();
        const auto glyph = bounds.reduced(bounds.getWidth() * kGlyphMargin, bounds.getHeight() * kGlyphMargin);

        // The drawable places itself inside glyph, so the turn is applied to the
        // context around it rather than to the drawable.
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(angle_, bounds.getCentreX(), bounds.getCentreY()));
        icon_->drawWithin(g, glyph, juce::RectanglePlacement::centred, 1.0f);
        g.restoreState();
    }

    void Spinner::visibilityChanged()
    {
        if (!isVisible())
        {
            stopTimer();
            return;
        }

        // A spinner that comes back starts its turn over, so a recycled one does
        // not appear at whatever angle the previous task left behind.
        angle_ = 0.0f;
        startTimerHz(kFramesPerSecond);
    }

    void Spinner::colourChanged()
    {
        repaint();
    }

    void Spinner::timerCallback()
    {
        angle_ += kRadiansPerFrame;

        if (angle_ >= juce::MathConstants<float>::twoPi)
        {
            angle_ -= juce::MathConstants<float>::twoPi;
        }

        repaint();
    }
}
