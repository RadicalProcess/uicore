#include "UICore/Waveform.h"
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

    Waveform::Waveform()
    : playheadPositionRatio_(0.0f)
    , playheadVisibility_(false)
    {
        setOpaque(true);
    }

    void Waveform::paint(juce::Graphics& g)
    {
        g.fillAll(juce::Colour(30, 30, 30));

        g.setColour(juce::Colour(60, 60, 60));
        g.drawRect(getLocalBounds(), 1);

        if (!waveformData_.empty())
        {
            paintWaveform(g);
            if (playheadVisibility_)
                paintPlaybackPosition(g);
        }
        else
        {
            g.setColour(juce::Colour(120, 120, 120));
            g.setFont(juce::FontOptions(14.0f));
            g.drawText("No audio file selected", getLocalBounds(), juce::Justification::centred);
        }
    }

    void Waveform::resized()
    {
        repaint();
    }

    void Waveform::setWaveformData(const std::vector<std::vector<float>>& waveformData)
    {
        waveformData_ = waveformData;
        repaint();
    }

    void Waveform::setPlayheadPosition(float positionRatio)
    {
        playheadPositionRatio_ = positionRatio;
        repaint();
    }

    void Waveform::setPlayheadVisibility(bool visibility)
    {
        playheadVisibility_ = visibility;
        repaint();
    }

    void Waveform::paintWaveform(juce::Graphics& g)
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

    void Waveform::paintPlaybackPosition(juce::Graphics& g) const
    {
        const auto bounds = getLocalBounds();
        const auto width = bounds.getWidth();
        const auto height = bounds.getHeight();
        const auto playbackX = playheadPositionRatio_ * width;
        g.setColour(styles::highlight);
        g.drawLine(playbackX, 0.0f, playbackX, static_cast<float>(height), 2.0f);
    }
}