#include "UICore/TransportView.h"

#include "UICore/Style.h"

namespace rp::uicore
{
    namespace
    {
        // Gap between the play/pause and stop buttons, in pixels.
        constexpr int kButtonSpacing = 4;
    }

    TransportView::TransportView()
    {
        playPauseButton_.onClick = [this]
        {
            if (onPlayPause)
                onPlayPause(playPauseButton_.getToggleState());
        };

        stopButton_.onClick = [this]
        {
            if (onStop)
                onStop();
        };

        addAndMakeVisible(playPauseButton_);
        addAndMakeVisible(stopButton_);
    }

    void TransportView::paint(juce::Graphics& g)
    {
        g.fillAll(styles::background);
    }

    void TransportView::resized()
    {
        // Round buttons the height of the strip, tucked into the left corner.
        auto area = getLocalBounds();
        const auto buttonSize = area.getHeight();

        playPauseButton_.setBounds(area.removeFromLeft(buttonSize));
        area.removeFromLeft(kButtonSpacing);
        stopButton_.setBounds(area.removeFromLeft(buttonSize));
    }
}
