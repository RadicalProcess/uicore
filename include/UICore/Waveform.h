#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace rp::uicore
{

    class Waveform : public juce::Component
    {
    public:
        class Listener
        {
        public:
            virtual ~Listener() = default;
            virtual void waveformSelectionStarted(float normalizedX) = 0;
            virtual void waveformSelectionDragged(float normalizedX) = 0;
            virtual void waveformSelectionEnded(float normalizedX) = 0;
        };

        Waveform();
        ~Waveform() override = default;

        void setWaveformData(const std::vector<std::vector<float>>& waveformData,
                             float durationInSeconds = 0.0f);
        void clearWaveform();
        void setSelection(float startRatio, float endRatio);
        void clearSelection();
        void setPlaybackPosition(float positionRatio);
        void setSelectionEnabled(bool enabled);
        void setPlaybackPositionVisible(bool visible);

        void addListener(Listener* listener);
        void removeListener(Listener* listener);

    private:
        void paint(juce::Graphics& g) override;
        void resized() override;
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;

        void paintWaveform(juce::Graphics& g);
        void paintMonoWaveform(juce::Graphics& g, const juce::Rectangle<float>& bounds);
        void paintStereoWaveform(juce::Graphics& g, const juce::Rectangle<float>& bounds);
        void paintChannelWaveform(juce::Graphics& g, const std::vector<float>& channelData,
                                  const juce::Rectangle<float>& bounds, float centerY, float maxAmplitude,
                                  const juce::Colour& colour);
        void paintSelection(juce::Graphics& g);
        void paintTimeMarkers(juce::Graphics& g);
        void paintPlaybackPosition(juce::Graphics& g);

        std::vector<std::vector<float>> waveformData_;
        int numChannels_;
        float durationInSeconds_;
        bool hasValidWaveform_;

        bool hasSelection_;
        float selectionStartRatio_;
        float selectionEndRatio_;

        bool isSelecting_;
        float playbackPositionRatio_;
        bool selectionEnabled_;
        bool playbackPositionVisible_;

        juce::ListenerList<Listener> listeners_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Waveform)
    };

}