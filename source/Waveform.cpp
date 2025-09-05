#include "UICore/Waveform.h"
#include "UICore/Style.h"

namespace rp::uicore
{

    namespace
    {
        void paintChannelWaveform(juce::Graphics& g, const std::vector<float>& channelData, const juce::Rectangle<float>& bounds, const juce::Colour& color)
        {
            const auto width = bounds.getWidth();
            const auto height = bounds.getHeight();
            const auto centerY = height * 0.5f;
            const auto maxAmplitude = centerY;
            const auto numSamples = std::min(static_cast<size_t>(width), channelData.size());

            g.setColour(color);

            auto waveformPath = juce::Path();
            for (auto i = static_cast<size_t>(0); i < numSamples; ++i)
            {
                const auto amplitude = channelData[i] * maxAmplitude;
                const auto yTop = centerY - amplitude + bounds.getY();
                i == 0 ? waveformPath.startNewSubPath(static_cast<float>(i), yTop) : waveformPath.lineTo(static_cast<float>(i), yTop);
            }
            g.strokePath(waveformPath, juce::PathStrokeType(1.0f));
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
        if (waveformData_.size() == 1)
        {
            paintChannelWaveform(g, waveformData_[0], bounds, juce::Colour(100, 200, 255));
        }
        else if (waveformData_.size() == 2)
        {
            const auto halfHeight = bounds.getHeight() / 2;
            const auto leftChannelBounds = bounds.removeFromTop(halfHeight);
            const auto rightChannelBounds = bounds;

            paintChannelWaveform(g, waveformData_[0], leftChannelBounds, juce::Colour(100, 200, 255));
            paintChannelWaveform(g, waveformData_[1], rightChannelBounds, juce::Colour(255, 150, 100));
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