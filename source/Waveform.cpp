#include "UICore/Waveform.h"

namespace rp::uicore
{

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

        const auto bounds = getLocalBounds().toFloat();

        if (waveformData_.size() == 1)
        {
            paintMonoWaveform(g, bounds);
        }
        else if (waveformData_.size() == 2)
        {
            paintStereoWaveform(g, bounds);
        }
    }

    void Waveform::paintMonoWaveform(juce::Graphics& g, const juce::Rectangle<float>& bounds)
    {
        if (waveformData_.empty() || waveformData_[0].empty())
            return;

        const auto width = bounds.getWidth();
        const auto height = bounds.getHeight();
        const auto centerY = height * 0.5f;
        const auto maxAmplitude = height * 0.4f;

        g.setColour(juce::Colour(100, 200, 255));

        juce::Path waveformPath;
        const auto& channelData = waveformData_[0];

        for (size_t i = 0; i < channelData.size(); ++i)
        {
            const auto x = (static_cast<float>(i) / static_cast<float>(channelData.size() - 1)) * width;
            const auto amplitude = channelData[i] * maxAmplitude;
            const auto yTop = centerY - amplitude;

            if (i == 0)
            {
                waveformPath.startNewSubPath(x, yTop);
            }
            else
            {
                waveformPath.lineTo(x, yTop);
            }
        }

        for (int i = static_cast<int>(channelData.size()) - 1; i >= 0; --i)
        {
            const auto x = (static_cast<float>(i) / static_cast<float>(channelData.size() - 1)) * width;
            const auto amplitude = channelData[static_cast<size_t>(i)] * maxAmplitude;
            const auto yBottom = centerY + amplitude;
            waveformPath.lineTo(x, yBottom);
        }

        waveformPath.closeSubPath();
        g.strokePath(waveformPath, juce::PathStrokeType(1.0f));

        g.setColour(juce::Colour(150, 220, 255));
        g.drawLine(0.0f, centerY, width, centerY, 1.0f);
    }

    void Waveform::paintStereoWaveform(juce::Graphics& g, const juce::Rectangle<float>& bounds)
    {
        if (waveformData_.size() < 2 || waveformData_[0].empty() || waveformData_[1].empty())
            return;

        const auto width = bounds.getWidth();
        const auto height = bounds.getHeight();
        const auto channelHeight = height * 0.5f;
        const auto maxAmplitude = channelHeight * 0.8f;

        const auto leftChannelCenterY = channelHeight * 0.5f;
        const auto rightChannelCenterY = channelHeight * 1.5f;

        paintChannelWaveform(g, waveformData_[0], bounds, leftChannelCenterY, maxAmplitude,
                             juce::Colour(100, 200, 255));
        paintChannelWaveform(g, waveformData_[1], bounds, rightChannelCenterY, maxAmplitude,
                             juce::Colour(255, 150, 100));

        g.setColour(juce::Colour(100, 100, 100));
        g.drawLine(0.0f, channelHeight, width, channelHeight, 1.0f);

        g.setColour(juce::Colour(150, 220, 255));
        g.drawLine(0.0f, leftChannelCenterY, width, leftChannelCenterY, 0.5f);

        g.setColour(juce::Colour(255, 180, 150));
        g.drawLine(0.0f, rightChannelCenterY, width, rightChannelCenterY, 0.5f);
    }

    void Waveform::paintChannelWaveform(juce::Graphics& g, const std::vector<float>& channelData,
                                        const juce::Rectangle<float>& bounds, float centerY, float maxAmplitude,
                                        const juce::Colour& colour)
    {
        if (channelData.empty())
            return;

        const auto width = bounds.getWidth();

        g.setColour(colour);

        juce::Path waveformPath;

        for (size_t i = 0; i < channelData.size(); ++i)
        {
            const auto x = (static_cast<float>(i) / static_cast<float>(channelData.size() - 1)) * width;
            const auto amplitude = channelData[i] * maxAmplitude;
            const auto yTop = centerY - amplitude;

            if (i == 0)
            {
                waveformPath.startNewSubPath(x, yTop);
            }
            else
            {
                waveformPath.lineTo(x, yTop);
            }
        }

        for (int i = static_cast<int>(channelData.size()) - 1; i >= 0; --i)
        {
            const auto x = (static_cast<float>(i) / static_cast<float>(channelData.size() - 1)) * width;
            const auto amplitude = channelData[static_cast<size_t>(i)] * maxAmplitude;
            const auto yBottom = centerY + amplitude;
            waveformPath.lineTo(x, yBottom);
        }

        waveformPath.closeSubPath();
        g.strokePath(waveformPath, juce::PathStrokeType(1.0f));
    }

    void Waveform::paintPlaybackPosition(juce::Graphics& g) const
    {
        const auto bounds = getLocalBounds();
        const auto width = bounds.getWidth();
        const auto height = bounds.getHeight();
        const auto playbackX = playheadPositionRatio_ * width;
        g.setColour(juce::Colours::red.withAlpha(0.8f));
        g.drawLine(playbackX, 0.0f, playbackX, static_cast<float>(height), 2.0f);
    }

}