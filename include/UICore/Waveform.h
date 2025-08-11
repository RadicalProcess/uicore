#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <optional>
#include <vector>

namespace rp::uicore
{

    class Waveform : public juce::Component
    {
    public:
        Waveform();
        ~Waveform() override = default;

        void setWaveformData(const std::vector<std::vector<float>>& waveformData);

        void setPlayheadPosition(float positionRatio);

        void setPlayheadVisibility(bool visible);
    private:
        void paint(juce::Graphics& g) override;
        void resized() override;

        void paintWaveform(juce::Graphics& g);
        void paintMonoWaveform(juce::Graphics& g, const juce::Rectangle<float>& bounds);
        void paintStereoWaveform(juce::Graphics& g, const juce::Rectangle<float>& bounds);
        void paintChannelWaveform(juce::Graphics& g, const std::vector<float>& channelData,
                                  const juce::Rectangle<float>& bounds, float centerY, float maxAmplitude,
                                  const juce::Colour& colour);
        void paintPlaybackPosition(juce::Graphics& g) const;

        std::vector<std::vector<float>> waveformData_;
        float playheadPositionRatio_;
        bool playheadVisibility_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Waveform)
    };

}