#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

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
        void paintPlaybackPosition(juce::Graphics& g) const;

        std::vector<std::vector<float>> waveformData_;
        float playheadPositionRatio_;
        bool playheadVisibility_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Waveform)
    };

}