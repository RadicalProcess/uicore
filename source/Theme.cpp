#include "Theme.h"

#include "Font.h"

namespace rp::uicore::theme
{
    namespace
    {
        constexpr float kPanelCornerRadius = 6.0f;
        constexpr float kPanelBorderThickness = 1.0f;
        // The sketch letter-spaces its section titles by 1 px at a 9 px size,
        // and JUCE takes the extra kerning as a fraction of the font height.
        constexpr float kSectionTitleKerning = 1.0f / 9.0f;

        // Opacity of a meter's fill over its track. The bright accent reads too
        // hot at full strength on a readout that is lit most of the time.
        constexpr float kMeterLevelOpacity = 0.85f;
    }

    const juce::Colour appBackground{0xff0d1016};
    const juce::Colour headerBar{0xff1a2029};
    const juce::Colour panelFill{0xff141821};
    const juce::Colour panelBorder{0xff3d4a5e};
    const juce::Colour insetWell{0xff080b10};
    const juce::Colour selectedFill{0xff182338};
    const juce::Colour selectedStroke{0xff3a6ad8};
    const juce::Colour accent{0xff3a6ad8};
    const juce::Colour accentBright{0xff7ec8ff};
    const juce::Colour waveformTrace{0xff465262};
    const juce::Colour textPrimary{0xffccd4de};
    const juce::Colour textSecondary{0xff75818f};
    const juce::Colour textHint{0xff525c68};
    const juce::Colour iconFrame{0xff2e3949};
    const juce::Colour iconStroke{0xffd2e6f8};
    const juce::Colour viewFrame{0xff28313f};
    const juce::Colour curve{0xffe4ecf4};
    const juce::Colour guide{0xff27303e};
    const juce::Colour controlWell{0xff0d1118};
    const juce::Colour overlayDim{0xcc0d1016};

    const int bodyMargin = 6;
    const int panelGap = 8;

    juce::Font makeFont(float height)
    {
        return getRobotoCondensed().withHeight(height);
    }

    void drawPanel(juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        const auto area = bounds.toFloat().reduced(kPanelBorderThickness * 0.5f);

        g.setColour(panelFill);
        g.fillRoundedRectangle(area, kPanelCornerRadius);

        g.setColour(panelBorder);
        g.drawRoundedRectangle(area, kPanelCornerRadius, kPanelBorderThickness);
    }

    void drawPanelHeader(juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        const auto area = bounds.toFloat().reduced(kPanelBorderThickness * 0.5f);

        juce::Path path;
        path.addRoundedRectangle(area.getX(), area.getY(), area.getWidth(), area.getHeight(), kPanelCornerRadius,
                                 kPanelCornerRadius, true, true, false, false);

        g.setColour(headerBar);
        g.fillPath(path);

        g.setColour(panelBorder);
        g.drawLine(area.getX(), area.getBottom(), area.getRight(), area.getBottom(), kPanelBorderThickness);
    }

    void drawViewPanel(juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        const auto area = bounds.toFloat().reduced(kPanelBorderThickness * 0.5f);

        g.setColour(insetWell);
        g.fillRoundedRectangle(area, kPanelCornerRadius);

        g.setColour(panelBorder);
        g.drawRoundedRectangle(area, kPanelCornerRadius, kPanelBorderThickness);
    }

    void styleSectionTitle(juce::Label& label)
    {
        label.setFont(getRobotoCondensed().withExtraKerningFactor(kSectionTitleKerning));
        label.setColour(juce::Label::textColourId, textSecondary);
    }

    void styleKnob(juce::Slider& slider)
    {
        slider.setColour(juce::Slider::rotarySliderFillColourId, controlWell);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, iconFrame);
        slider.setColour(juce::Slider::thumbColourId, textPrimary);
        slider.setColour(juce::Slider::textBoxTextColourId, textPrimary);
    }

    void styleMeter(LevelIndicator& meter)
    {
        meter.setColour(LevelIndicator::trackColourId, controlWell);
        meter.setColour(LevelIndicator::levelColourId, accentBright.withAlpha(kMeterLevelOpacity));
    }

    void styleWaveform(Waveform& waveform)
    {
        waveform.setColour(Waveform::backgroundColourId, controlWell);
        waveform.setColour(Waveform::outlineColourId, panelBorder);
        waveform.setColour(Waveform::traceColourId, waveformTrace);
        waveform.setColour(Waveform::playheadColourId, accentBright);
        waveform.setColour(Waveform::selectionColourId, accentBright);
        waveform.setColour(Waveform::fadeColourId, accentBright);
        waveform.setColour(Waveform::placeholderTextColourId, textHint);
    }

    void styleWaveformScrollBar(WaveformScrollBar& scrollBar)
    {
        scrollBar.setColour(WaveformScrollBar::backgroundColourId, insetWell);
        scrollBar.setColour(WaveformScrollBar::outlineColourId, panelBorder);
        scrollBar.setColour(WaveformScrollBar::traceColourId, waveformTrace);
        scrollBar.setColour(WaveformScrollBar::thumbColourId, accent);
        scrollBar.setColour(WaveformScrollBar::markerColourId, accentBright);
    }
}
