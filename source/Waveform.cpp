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

        // Alpha applied to the highlight colour for the translucent selection
        // overlay drawn on top of the waveform.
        const auto selectionAlpha_ = 0.3f;

        // Alpha applied to the highlight colour for the translucent triangle
        // covering the attenuated part of a fade.
        const auto fadeAlpha_ = 0.25f;

        // Half-width and height (in pixels) of the triangle fade handles drawn
        // at the top corners of the selection.
        const auto fadeHandleHalfWidth_ = 6.0f;
        const auto fadeHandleHeight_ = 9.0f;

        // Horizontal slack (in pixels) added either side of a handle when hit
        // testing, so the small triangles are comfortable to grab.
        const auto fadeHandleHitMargin_ = 4.0f;

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

    Waveform::Waveform()
    : playheadPositionRatio_(0.0f)
    , playheadVisibility_(false)
    , selectionEnabled_(false)
    , selectionStartRatio_(0.0f)
    , selectionEndRatio_(0.0f)
    , hasSelection_(false)
    , fadeEnabled_(false)
    , fadeInRatio_(0.0f)
    , fadeOutRatio_(0.0f)
    , activeFadeHandle_(FadeHandle::None)
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
            if (hasSelection_)
                paintSelection(g);
            if (fadeHandlesVisible())
                paintFades(g);
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

    void Waveform::setSelectionEnabled(bool enabled)
    {
        if (selectionEnabled_ == enabled)
            return;

        selectionEnabled_ = enabled;

        // Disabling drops any cached selection so a later re-enable starts from
        // a clean slate rather than re-exposing a stale region.
        if (!selectionEnabled_)
            clearSelection();
    }

    void Waveform::setSelection(float startRatio, float endRatio)
    {
        // The programmatic setter is also gated: when selection is disabled the
        // component holds no region at all.
        if (!selectionEnabled_)
            return;

        const auto clampedStart = std::clamp(startRatio, 0.0f, 1.0f);
        const auto clampedEnd = std::clamp(endRatio, 0.0f, 1.0f);

        selectionStartRatio_ = std::min(clampedStart, clampedEnd);
        selectionEndRatio_ = std::max(clampedStart, clampedEnd);
        hasSelection_ = true;
        resetFades();

        notifySelectionChanged();
        repaint();
    }

    void Waveform::clearSelection()
    {
        if (!hasSelection_)
            return;

        hasSelection_ = false;
        selectionStartRatio_ = 0.0f;
        selectionEndRatio_ = 0.0f;
        activeFadeHandle_ = FadeHandle::None;
        resetFades();

        notifySelectionChanged();
        repaint();
    }

    bool Waveform::hasSelection() const
    {
        return hasSelection_;
    }

    float Waveform::getSelectionStart() const
    {
        return selectionStartRatio_;
    }

    float Waveform::getSelectionEnd() const
    {
        return selectionEndRatio_;
    }

    void Waveform::setFadeEnabled(bool enabled)
    {
        if (fadeEnabled_ == enabled)
            return;

        fadeEnabled_ = enabled;

        // Disabling drops any cached fades and aborts an in-progress drag so a
        // later re-enable starts without stale slopes.
        if (!fadeEnabled_)
        {
            activeFadeHandle_ = FadeHandle::None;
            resetFades();
        }

        repaint();
    }

    float Waveform::getFadeIn() const
    {
        return fadeInRatio_;
    }

    float Waveform::getFadeOut() const
    {
        return fadeOutRatio_;
    }

    void Waveform::mouseDown(const juce::MouseEvent& event)
    {
        // A fade handle takes priority over starting a new selection so the user
        // can adjust a fade without disturbing the region underneath it.
        if (fadeHandlesVisible())
        {
            const auto handle = fadeHandleAt(event.getPosition());
            if (handle != FadeHandle::None)
            {
                activeFadeHandle_ = handle;
                return;
            }
        }

        if (!selectionEnabled_)
            return;

        // Begin a new selection anchored at the click position. Until the user
        // drags, start and end are identical (an empty selection).
        const auto ratio = ratioForX(event.x);
        selectionStartRatio_ = ratio;
        selectionEndRatio_ = ratio;
        hasSelection_ = true;
        resetFades();

        notifySelectionChanged();
        repaint();
    }

    void Waveform::mouseDrag(const juce::MouseEvent& event)
    {
        if (activeFadeHandle_ != FadeHandle::None)
        {
            const auto leftX = selectionLeftX();
            const auto rightX = selectionRightX();
            const auto width = rightX - leftX;
            if (width <= 0.0f)
                return;

            const auto pointerX = std::clamp(static_cast<float>(event.x), leftX, rightX);

            if (activeFadeHandle_ == FadeHandle::In)
            {
                // Fade-in grows from the left edge but may not reach past the
                // start of the fade-out.
                const auto maxRatio = 1.0f - fadeOutRatio_;
                fadeInRatio_ = std::clamp((pointerX - leftX) / width, 0.0f, maxRatio);
            }
            else
            {
                // Fade-out grows from the right edge but may not reach past the
                // end of the fade-in.
                const auto maxRatio = 1.0f - fadeInRatio_;
                fadeOutRatio_ = std::clamp((rightX - pointerX) / width, 0.0f, maxRatio);
            }

            notifyFadeChanged();
            repaint();
            return;
        }

        if (!selectionEnabled_)
            return;

        // Extend the selection to the current pointer position. The drag may go
        // either side of the anchor, so order the ratios when reporting.
        selectionEndRatio_ = ratioForX(event.x);

        notifySelectionChanged();
        repaint();
    }

    void Waveform::mouseUp(const juce::MouseEvent& event)
    {
        if (activeFadeHandle_ != FadeHandle::None)
        {
            activeFadeHandle_ = FadeHandle::None;
            notifyFadeChanged();
            return;
        }

        if (!selectionEnabled_)
            return;

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

    void Waveform::paintSelection(juce::Graphics& g) const
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

    void Waveform::paintFades(juce::Graphics& g) const
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto top = bounds.getY();
        const auto bottom = bounds.getBottom();
        const auto leftX = selectionLeftX();
        const auto rightX = selectionRightX();
        const auto width = rightX - leftX;
        if (width <= 0.0f)
            return;

        const auto fadeColour = styles::highlight;

        // The slope rises from silence (bottom) at the selection edge to full
        // gain (top) where the fade ends; the triangle it cuts off is shaded to
        // emphasise the attenuated portion.
        const auto fadeInEndX = leftX + fadeInRatio_ * width;
        {
            juce::Path covered;
            covered.startNewSubPath(leftX, top);
            covered.lineTo(fadeInEndX, top);
            covered.lineTo(leftX, bottom);
            covered.closeSubPath();

            g.setColour(fadeColour.withAlpha(fadeAlpha_));
            g.fillPath(covered);

            g.setColour(fadeColour);
            g.drawLine(leftX, bottom, fadeInEndX, top, 1.5f);
        }

        const auto fadeOutStartX = rightX - fadeOutRatio_ * width;
        {
            juce::Path covered;
            covered.startNewSubPath(fadeOutStartX, top);
            covered.lineTo(rightX, top);
            covered.lineTo(rightX, bottom);
            covered.closeSubPath();

            g.setColour(fadeColour.withAlpha(fadeAlpha_));
            g.fillPath(covered);

            g.setColour(fadeColour);
            g.drawLine(fadeOutStartX, top, rightX, bottom, 1.5f);
        }

        // Draw the grab handles last so they sit on top of the slopes. Each is a
        // small triangle hanging from the top edge at the fade boundary.
        g.setColour(fadeColour);
        for (const auto handleX : { fadeInEndX, fadeOutStartX })
        {
            juce::Path handle;
            handle.startNewSubPath(handleX - fadeHandleHalfWidth_, top);
            handle.lineTo(handleX + fadeHandleHalfWidth_, top);
            handle.lineTo(handleX, top + fadeHandleHeight_);
            handle.closeSubPath();
            g.fillPath(handle);
        }
    }

    float Waveform::ratioForX(int x) const
    {
        const auto width = getLocalBounds().getWidth();
        if (width <= 0)
            return 0.0f;

        return std::clamp(static_cast<float>(x) / static_cast<float>(width), 0.0f, 1.0f);
    }

    void Waveform::notifySelectionChanged() const
    {
        if (!onSelectionChanged)
            return;

        const auto startRatio = std::min(selectionStartRatio_, selectionEndRatio_);
        const auto endRatio = std::max(selectionStartRatio_, selectionEndRatio_);
        onSelectionChanged(startRatio, endRatio);
    }

    void Waveform::notifyFadeChanged() const
    {
        if (onFadeChanged)
            onFadeChanged(fadeInRatio_, fadeOutRatio_);
    }

    float Waveform::selectionLeftX() const
    {
        const auto width = static_cast<float>(getLocalBounds().getWidth());
        const auto startRatio = std::min(selectionStartRatio_, selectionEndRatio_);
        return startRatio * width;
    }

    float Waveform::selectionRightX() const
    {
        const auto width = static_cast<float>(getLocalBounds().getWidth());
        const auto endRatio = std::max(selectionStartRatio_, selectionEndRatio_);
        return endRatio * width;
    }

    Waveform::FadeHandle Waveform::fadeHandleAt(juce::Point<int> point) const
    {
        if (!fadeHandlesVisible())
            return FadeHandle::None;

        const auto top = static_cast<float>(getLocalBounds().getY());
        const auto pointY = static_cast<float>(point.getY());
        if (pointY < top || pointY > top + fadeHandleHeight_)
            return FadeHandle::None;

        const auto leftX = selectionLeftX();
        const auto rightX = selectionRightX();
        const auto width = rightX - leftX;
        if (width <= 0.0f)
            return FadeHandle::None;

        const auto pointX = static_cast<float>(point.getX());
        const auto hitHalfWidth = fadeHandleHalfWidth_ + fadeHandleHitMargin_;

        const auto fadeInEndX = leftX + fadeInRatio_ * width;
        const auto fadeOutStartX = rightX - fadeOutRatio_ * width;

        // When both handles sit on top of each other (full selection covered, or
        // both fades at zero on a tiny selection) the in-handle wins; the
        // out-handle can still be reached once the in-handle is dragged away.
        if (std::abs(pointX - fadeInEndX) <= hitHalfWidth)
            return FadeHandle::In;
        if (std::abs(pointX - fadeOutStartX) <= hitHalfWidth)
            return FadeHandle::Out;

        return FadeHandle::None;
    }

    bool Waveform::fadeHandlesVisible() const
    {
        return fadeEnabled_ && hasSelection_;
    }

    void Waveform::resetFades()
    {
        fadeInRatio_ = 0.0f;
        fadeOutRatio_ = 0.0f;
    }
}
