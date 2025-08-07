#include "UICore/Waveform.h"

#include <algorithm>

namespace rp::uicore
{

    Waveform::Waveform()
    : numChannels_(0)
    , durationInSeconds_(0.0f)
    , hasValidWaveform_(false)
    , hasSelection_(false)
    , selectionStartRatio_(0.0f)
    , selectionEndRatio_(0.0f)
    , isSelecting_(false)
    , playbackPositionRatio_(0.0f)
    , selectionEnabled_(true)
    , playbackPositionVisible_(true)
    {
        setOpaque(true);
    }

    void Waveform::paint(juce::Graphics& g)
    {
        g.fillAll(juce::Colour(30, 30, 30));

        g.setColour(juce::Colour(60, 60, 60));
        g.drawRect(getLocalBounds(), 1);

        if (hasValidWaveform_)
        {
            paintWaveform(g);
            paintSelection(g);
            if (playbackPositionVisible_)
                paintPlaybackPosition(g);
            paintTimeMarkers(g);
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

    void Waveform::mouseDown(const juce::MouseEvent& event)
    {
        if (!hasValidWaveform_ || !selectionEnabled_)
            return;

        isSelecting_ = true;

        const auto normalizedX = static_cast<float>(event.x) / static_cast<float>(getWidth());
        listeners_.call([normalizedX](Listener& l) {
            l.waveformSelectionStarted(normalizedX);
        });
    }

    void Waveform::mouseDrag(const juce::MouseEvent& event)
    {
        if (!isSelecting_ || !hasValidWaveform_ || !selectionEnabled_)
            return;

        const auto normalizedX = static_cast<float>(event.x) / static_cast<float>(getWidth());
        listeners_.call([normalizedX](Listener& l) {
            l.waveformSelectionDragged(normalizedX);
        });
    }

    void Waveform::mouseUp(const juce::MouseEvent& event)
    {
        if (!isSelecting_)
            return;

        isSelecting_ = false;

        const auto normalizedX = static_cast<float>(event.x) / static_cast<float>(getWidth());
        listeners_.call([normalizedX](Listener& l) {
            l.waveformSelectionEnded(normalizedX);
        });
    }

    void Waveform::addListener(Listener* listener)
    {
        listeners_.add(listener);
    }

    void Waveform::removeListener(Listener* listener)
    {
        listeners_.remove(listener);
    }

    void Waveform::setWaveformData(const std::vector<std::vector<float>>& waveformData,
                                   float durationInSeconds)
    {
        waveformData_ = waveformData;
        numChannels_ = static_cast<int>(waveformData.size());
        durationInSeconds_ = durationInSeconds;
        hasValidWaveform_ = !waveformData_.empty() && numChannels_ > 0;
        repaint();
    }

    void Waveform::clearWaveform()
    {
        waveformData_.clear();
        numChannels_ = 0;
        durationInSeconds_ = 0.0f;
        hasValidWaveform_ = false;
        playbackPositionRatio_ = 0.0f;
        clearSelection();
        repaint();
    }

    void Waveform::setSelection(float startRatio, float endRatio)
    {
        selectionStartRatio_ = std::min(startRatio, endRatio);
        selectionEndRatio_ = std::max(startRatio, endRatio);
        hasSelection_ = (selectionEndRatio_ > selectionStartRatio_);
        repaint();
    }

    void Waveform::clearSelection()
    {
        hasSelection_ = false;
        selectionStartRatio_ = 0.0f;
        selectionEndRatio_ = 0.0f;
        repaint();
    }

    void Waveform::setPlaybackPosition(float positionRatio)
    {
        playbackPositionRatio_ = positionRatio;
        repaint();
    }

    void Waveform::setSelectionEnabled(bool enabled)
    {
        selectionEnabled_ = enabled;
    }

    void Waveform::setPlaybackPositionVisible(bool visible)
    {
        playbackPositionVisible_ = visible;
        repaint();
    }

    void Waveform::paintWaveform(juce::Graphics& g)
    {
        if (waveformData_.empty() || numChannels_ == 0)
            return;

        const auto bounds = getLocalBounds().toFloat();

        if (numChannels_ == 1)
        {
            paintMonoWaveform(g, bounds);
        }
        else if (numChannels_ == 2)
        {
            paintStereoWaveform(g, bounds);
        }
        else
        {
            paintMonoWaveform(g, bounds);
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
        g.fillPath(waveformPath);

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
        g.fillPath(waveformPath);
    }

    void Waveform::paintSelection(juce::Graphics& g)
    {
        if (!hasSelection_)
            return;

        const auto bounds = getLocalBounds().toFloat();
        const auto width = bounds.getWidth();
        const auto startX = selectionStartRatio_ * width;
        const auto endX = selectionEndRatio_ * width;
        const auto selectionWidth = endX - startX;

        if (selectionWidth < 1.0f)
            return;

        const auto selectionBounds = juce::Rectangle<float>(startX, 0.0f, selectionWidth, bounds.getHeight());

        g.setColour(juce::Colours::yellow.withAlpha(0.3f));
        g.fillRect(selectionBounds);

        g.setColour(juce::Colours::yellow.withAlpha(0.6f));
        g.drawRect(selectionBounds, 1.0f);
    }

    void Waveform::paintTimeMarkers(juce::Graphics& g)
    {
        if (durationInSeconds_ <= 0.0f)
            return;

        const auto bounds = getLocalBounds();
        const auto width = bounds.getWidth();
        const auto height = bounds.getHeight();

        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.setFont(juce::FontOptions(14.0f));

        const auto leftTime = juce::String(0.0f, 1) + "s";
        const auto middleTime = juce::String(durationInSeconds_ * 0.5f, 1) + "s";
        const auto rightTime = juce::String(durationInSeconds_, 1) + "s";

        const auto textHeight = 16;
        const auto yPosition = height - textHeight - 3;
        const auto margin = 5;

        juce::GlyphArrangement glyphs;

        glyphs.addLineOfText(g.getCurrentFont(), leftTime, 0.0f, 0.0f);
        const auto leftTextWidth = static_cast<int>(glyphs.getBoundingBox(0, -1, true).getWidth()) + 4;
        g.drawText(leftTime, margin, yPosition, leftTextWidth, textHeight, juce::Justification::left);

        glyphs.clear();
        glyphs.addLineOfText(g.getCurrentFont(), middleTime, 0.0f, 0.0f);
        const auto middleTextWidth = static_cast<int>(glyphs.getBoundingBox(0, -1, true).getWidth()) + 4;
        g.drawText(middleTime, (width - middleTextWidth) / 2, yPosition, middleTextWidth, textHeight,
                   juce::Justification::centred);

        glyphs.clear();
        glyphs.addLineOfText(g.getCurrentFont(), rightTime, 0.0f, 0.0f);
        const auto rightTextWidth = static_cast<int>(glyphs.getBoundingBox(0, -1, true).getWidth()) + 4;
        g.drawText(rightTime, width - rightTextWidth - margin, yPosition, rightTextWidth, textHeight,
                   juce::Justification::right);
    }

    void Waveform::paintPlaybackPosition(juce::Graphics& g)
    {
        if (playbackPositionRatio_ <= 0.0f)
            return;

        const auto bounds = getLocalBounds();
        const auto width = bounds.getWidth();
        const auto height = bounds.getHeight();

        float playbackX;

        if (hasSelection_)
        {
            // DEBUG: Let's see what values we're working with
            const auto selectionStart = selectionStartRatio_ * width;
            const auto selectionEnd = selectionEndRatio_ * width;
            const auto selectionWidth = selectionEnd - selectionStart;

            // The problem might be that playbackPositionRatio_ is still global position
            // Let's try a different approach: if playbackPositionRatio_ is global,
            // we need to check if it's within selection bounds and remap it

            if (playbackPositionRatio_ >= selectionStartRatio_ && playbackPositionRatio_ <= selectionEndRatio_)
            {
                // playbackPositionRatio_ appears to be global - remap to selection
                const auto globalToSelectionRatio =
                    (playbackPositionRatio_ - selectionStartRatio_) / (selectionEndRatio_ - selectionStartRatio_);
                playbackX = selectionStart + (globalToSelectionRatio * selectionWidth);
            }
            else
            {
                // Assume it's already selection-relative (original logic)
                playbackX = selectionStart + (playbackPositionRatio_ * selectionWidth);
            }
        }
        else
        {
            // For full waveform playback
            playbackX = playbackPositionRatio_ * width;
        }

        g.setColour(juce::Colours::red.withAlpha(0.8f));
        g.drawLine(playbackX, 0.0f, playbackX, static_cast<float>(height), 2.0f);
    }

}