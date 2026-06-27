#include "UICore/WaveformSelector.h"
#include "UICore/Style.h"

#include <algorithm>
#include <cmath>

namespace rp::uicore
{

    namespace
    {
        // Each waveform is drawn as a series of evenly spaced vertical lines.
        // lineWidth_ is the thickness of one line, lineStride_ is the horizontal
        // distance between the start of one line and the next (line + gap).
        const auto lineWidth_ = 1.0f;
        const auto lineStride_ = 3.0f;

        // Alpha applied to the highlight colour for the translucent selection
        // overlay drawn on top of the waveform.
        const auto selectionAlpha_ = 0.3f;

        void paintChannelWaveform(juce::Graphics& g, const std::vector<float>& channelData, const juce::Rectangle<float>& bounds, const juce::Colour& color)
        {
            if (channelData.empty())
                return;

            const auto centerY = bounds.getCentreY();
            const auto maxAmplitude = bounds.getHeight() * 0.5f;
            const auto numLines = static_cast<size_t>(bounds.getWidth() / lineStride_);

            if (numLines == 0)
                return;

            g.setColour(color);

            for (auto i = static_cast<size_t>(0); i < numLines; ++i)
            {
                // Map this line to a contiguous bucket of samples and take the
                // loudest magnitude in it, so the line length reflects the peak
                // of the audio it represents rather than a single sample.
                const auto bucketStart = i * channelData.size() / numLines;
                const auto bucketEnd = std::max(bucketStart + 1, (i + 1) * channelData.size() / numLines);

                auto peak = 0.0f;
                for (auto s = bucketStart; s < bucketEnd && s < channelData.size(); ++s)
                    peak = std::max(peak, std::abs(channelData[s]));

                // Half-length above and below the centre line: peak (0..1) scaled
                // to half the available height. Total line length = 2 * halfLength.
                const auto halfLength = peak * maxAmplitude;
                const auto x = bounds.getX() + static_cast<float>(i) * lineStride_;
                g.drawLine(x, centerY - halfLength, x, centerY + halfLength, lineWidth_);
            }
        }
    }

    WaveformSelector::WaveformSelector()
    : selectionStartRatio_(0.0f)
    , selectionEndRatio_(0.0f)
    , hasSelection_(false)
    {
        setOpaque(true);
    }

    void WaveformSelector::paint(juce::Graphics& g)
    {
        g.fillAll(juce::Colour(30, 30, 30));

        g.setColour(juce::Colour(60, 60, 60));
        g.drawRect(getLocalBounds(), 1);

        if (!waveformData_.empty())
        {
            paintWaveform(g);
            if (hasSelection_)
                paintSelection(g);
        }
        else
        {
            g.setColour(juce::Colour(120, 120, 120));
            g.setFont(juce::FontOptions(14.0f));
            g.drawText("No audio file selected", getLocalBounds(), juce::Justification::centred);
        }
    }

    void WaveformSelector::resized()
    {
        repaint();
    }

    void WaveformSelector::setWaveformData(const std::vector<std::vector<float>>& waveformData)
    {
        waveformData_ = waveformData;
        repaint();
    }

    void WaveformSelector::setSelection(float startRatio, float endRatio)
    {
        const auto clampedStart = std::clamp(startRatio, 0.0f, 1.0f);
        const auto clampedEnd = std::clamp(endRatio, 0.0f, 1.0f);

        selectionStartRatio_ = std::min(clampedStart, clampedEnd);
        selectionEndRatio_ = std::max(clampedStart, clampedEnd);
        hasSelection_ = true;

        notifySelectionChanged();
        repaint();
    }

    void WaveformSelector::clearSelection()
    {
        if (!hasSelection_)
            return;

        hasSelection_ = false;
        selectionStartRatio_ = 0.0f;
        selectionEndRatio_ = 0.0f;

        notifySelectionChanged();
        repaint();
    }

    bool WaveformSelector::hasSelection() const
    {
        return hasSelection_;
    }

    float WaveformSelector::getSelectionStart() const
    {
        return selectionStartRatio_;
    }

    float WaveformSelector::getSelectionEnd() const
    {
        return selectionEndRatio_;
    }

    void WaveformSelector::mouseDown(const juce::MouseEvent& event)
    {
        // Begin a new selection anchored at the click position. Until the user
        // drags, start and end are identical (an empty selection).
        const auto ratio = ratioForX(event.x);
        selectionStartRatio_ = ratio;
        selectionEndRatio_ = ratio;
        hasSelection_ = true;

        notifySelectionChanged();
        repaint();
    }

    void WaveformSelector::mouseDrag(const juce::MouseEvent& event)
    {
        // Extend the selection to the current pointer position. The drag may go
        // either side of the anchor, so order the ratios when reporting.
        selectionEndRatio_ = ratioForX(event.x);

        notifySelectionChanged();
        repaint();
    }

    void WaveformSelector::mouseUp(const juce::MouseEvent& event)
    {
        // A click without a drag leaves an empty (zero-width) selection, which
        // is not a useful region; treat it as clearing the selection.
        if (selectionStartRatio_ == selectionEndRatio_)
        {
            clearSelection();
            return;
        }

        selectionEndRatio_ = ratioForX(event.x);

        notifySelectionChanged();
        repaint();
    }

    void WaveformSelector::paintWaveform(juce::Graphics& g)
    {
        if (waveformData_.empty())
            return;

        auto bounds = getLocalBounds().toFloat();
        const auto channelColor = styles::foreground;
        if (waveformData_.size() == 1)
        {
            paintChannelWaveform(g, waveformData_[0], bounds, channelColor);
        }
        else if (waveformData_.size() == 2)
        {
            const auto halfHeight = bounds.getHeight() / 2;
            const auto leftChannelBounds = bounds.removeFromTop(halfHeight);
            const auto rightChannelBounds = bounds;

            paintChannelWaveform(g, waveformData_[0], leftChannelBounds, channelColor);
            paintChannelWaveform(g, waveformData_[1], rightChannelBounds, channelColor);
        }
    }

    void WaveformSelector::paintSelection(juce::Graphics& g) const
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto width = bounds.getWidth();

        // Order the ratios so the overlay is drawn correctly regardless of the
        // drag direction.
        const auto startRatio = std::min(selectionStartRatio_, selectionEndRatio_);
        const auto endRatio = std::max(selectionStartRatio_, selectionEndRatio_);

        const auto startX = bounds.getX() + startRatio * width;
        const auto endX = bounds.getX() + endRatio * width;

        const auto selectionBounds = juce::Rectangle<float>(startX, bounds.getY(), endX - startX, bounds.getHeight());

        g.setColour(styles::highlight.withAlpha(selectionAlpha_));
        g.fillRect(selectionBounds);

        g.setColour(styles::highlight);
        g.drawLine(startX, bounds.getY(), startX, bounds.getBottom(), 1.0f);
        g.drawLine(endX, bounds.getY(), endX, bounds.getBottom(), 1.0f);
    }

    float WaveformSelector::ratioForX(int x) const
    {
        const auto width = getLocalBounds().getWidth();
        if (width <= 0)
            return 0.0f;

        return std::clamp(static_cast<float>(x) / static_cast<float>(width), 0.0f, 1.0f);
    }

    void WaveformSelector::notifySelectionChanged() const
    {
        if (!onSelectionChanged)
            return;

        const auto startRatio = std::min(selectionStartRatio_, selectionEndRatio_);
        const auto endRatio = std::max(selectionStartRatio_, selectionEndRatio_);
        onSelectionChanged(startRatio, endRatio);
    }
}
