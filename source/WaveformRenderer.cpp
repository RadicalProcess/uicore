#include "UICore/WaveformRenderer.h"

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

        // The renderer draws exactly one of three mutually exclusive
        // representations, chosen by the samples-per-pixel (SPP) ratio as the
        // waveform gets denser:
        //   SPP <= sampleDotMaxSpp_     : connected line WITH a dot per sample.
        //   SPP <= connectedLineMaxSpp_ : connected line only.
        //   otherwise                   : peak vertical lines only.
        // The dot threshold is the smallest of the three so the dots only ever
        // decorate the connected line and never the peak lines; it is kept low
        // enough that the dots stay separated rather than merging into a line.
        const auto sampleDotMaxSpp_ = 0.2f;
        const auto connectedLineMaxSpp_ = 2.0f;
        const auto sampleDotRadius_ = 2.0f;

        // Horizontal position of a sample, spreading the samples evenly across
        // the full width so the first and last samples sit on the edges.
        float sampleX(const juce::Rectangle<float>& bounds, size_t index, size_t numSamples)
        {
            if (numSamples <= 1)
                return bounds.getX();

            return bounds.getX() + bounds.getWidth() * static_cast<float>(index) / static_cast<float>(numSamples - 1);
        }

        // Vertical position of a sample value (-1..1) relative to the centre
        // line, with positive amplitudes drawn upwards.
        float sampleY(float value, float centerY, float maxAmplitude)
        {
            return centerY - value * maxAmplitude;
        }

        // Peak rendering: one vertical line per pixel column, its length set by
        // the loudest sample in the bucket it represents. Suited to dense audio
        // where many samples map to each pixel.
        void paintPeakBars(juce::Graphics& g, const std::vector<float>& channelData, const juce::Rectangle<float>& bounds, float centerY, float maxAmplitude)
        {
            const auto numLines = static_cast<size_t>(bounds.getWidth() / lineStride_);
            if (numLines == 0)
                return;

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

        // Connected rendering: a polyline through every sample, showing the
        // actual waveform shape (and the interpolation between samples) when few
        // samples map to each pixel.
        void paintConnectedLine(juce::Graphics& g, const std::vector<float>& channelData, const juce::Rectangle<float>& bounds, float centerY, float maxAmplitude)
        {
            const auto numSamples = channelData.size();

            juce::Path path;
            for (auto i = static_cast<size_t>(0); i < numSamples; ++i)
            {
                const auto x = sampleX(bounds, i, numSamples);
                const auto y = sampleY(channelData[i], centerY, maxAmplitude);

                if (i == 0)
                    path.startNewSubPath(x, y);
                else
                    path.lineTo(x, y);
            }

            g.strokePath(path, juce::PathStrokeType(lineWidth_));
        }

        // Draws a small filled dot at each sample position so individual samples
        // remain visible on top of the connected line.
        void paintSampleDots(juce::Graphics& g, const std::vector<float>& channelData, const juce::Rectangle<float>& bounds, float centerY, float maxAmplitude)
        {
            const auto numSamples = channelData.size();
            const auto diameter = sampleDotRadius_ * 2.0f;

            for (auto i = static_cast<size_t>(0); i < numSamples; ++i)
            {
                const auto x = sampleX(bounds, i, numSamples);
                const auto y = sampleY(channelData[i], centerY, maxAmplitude);
                g.fillEllipse(x - sampleDotRadius_, y - sampleDotRadius_, diameter, diameter);
            }
        }

        void paintChannelWaveform(juce::Graphics& g, const std::vector<float>& channelData, const juce::Rectangle<float>& bounds, const juce::Colour& color)
        {
            if (channelData.empty())
                return;

            const auto width = bounds.getWidth();
            if (width <= 0.0f)
                return;

            const auto centerY = bounds.getCentreY();
            const auto maxAmplitude = bounds.getHeight() * 0.5f;
            const auto samplesPerPixel = static_cast<float>(channelData.size()) / width;

            g.setColour(color);

            // Dense audio collapses to peak vertical lines; nothing is drawn on
            // top of them so the three representations stay mutually exclusive.
            if (samplesPerPixel > connectedLineMaxSpp_)
            {
                paintPeakBars(g, channelData, bounds, centerY, maxAmplitude);
                return;
            }

            // Sparser audio shows its actual shape as a connected line, gaining
            // a dot per sample only once the samples are sparse enough to read
            // individually (so going to denser audio drops the dots, not the
            // line).
            paintConnectedLine(g, channelData, bounds, centerY, maxAmplitude);

            if (samplesPerPixel <= sampleDotMaxSpp_)
                paintSampleDots(g, channelData, bounds, centerY, maxAmplitude);
        }
    }

    WaveformRenderer::WaveformRenderer()
    : playheadPositionRatio_(0.0f)
    , playheadVisibility_(false)
    {
    }

    void WaveformRenderer::setWaveformData(const std::vector<std::vector<float>>& waveformData)
    {
        waveformData_ = waveformData;
    }

    bool WaveformRenderer::isEmpty() const
    {
        return waveformData_.empty();
    }

    void WaveformRenderer::setPlayheadPosition(float positionRatio)
    {
        playheadPositionRatio_ = positionRatio;
    }

    void WaveformRenderer::setPlayheadVisibility(bool visible)
    {
        playheadVisibility_ = visible;
    }

    bool WaveformRenderer::isPlayheadVisible() const
    {
        return playheadVisibility_;
    }

    void WaveformRenderer::paintWaveform(juce::Graphics& g, const juce::Rectangle<float>& bounds, const juce::Colour& colour) const
    {
        if (waveformData_.empty())
            return;

        if (waveformData_.size() == 1)
        {
            paintChannelWaveform(g, waveformData_[0], bounds, colour);
        }
        else if (waveformData_.size() == 2)
        {
            auto remaining = bounds;
            const auto halfHeight = remaining.getHeight() / 2;
            const auto leftChannelBounds = remaining.removeFromTop(halfHeight);
            const auto rightChannelBounds = remaining;

            paintChannelWaveform(g, waveformData_[0], leftChannelBounds, colour);
            paintChannelWaveform(g, waveformData_[1], rightChannelBounds, colour);
        }
    }

    void WaveformRenderer::paintPlayhead(juce::Graphics& g, const juce::Rectangle<float>& bounds, const juce::Colour& colour) const
    {
        if (!playheadVisibility_)
            return;

        const auto playbackX = bounds.getX() + playheadPositionRatio_ * bounds.getWidth();
        g.setColour(colour);
        g.drawLine(playbackX, bounds.getY(), playbackX, bounds.getBottom(), 2.0f);
    }

}
