#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "Knob.h"
#include "DisabledRenderer.h"
#include "EnabledRenderer.h"

namespace rp::uicore::glisson
{
    class Slider : public juce::Component
    {
    public:
        enum SliderGroup
        {
            StartPosition,
            EndPosition,
            StartPitch,
            EndPitch
        };

        class Listener
        {
        public:
            virtual ~Listener() = default;

            virtual void onSliderValueChanged(SliderGroup sliderGroup, float min, float max) = 0;
        };

    public:
        Slider();

        void setStartLeft(float value);
        void setStartRight(float value);
        void setEndLeft(float value);
        void setEndRight(float value);
        void setStartMin(float value);
        void setStartMax(float value);
        void setEndMin(float value);
        void setEndMax(float value);

    private:
        void resized() override;

        void paint(juce::Graphics& g) override;

        void mouseDown(const juce::MouseEvent& event) override;

        void mouseDrag(const juce::MouseEvent& event) override;

        void mouseUp(const juce::MouseEvent& event) override;

        std::vector<std::unique_ptr<Knob>> knobs_;
        bool enabled_;

        EnabledRenderer enabledRenderer_;
        DisabledRenderer disabledRenderer_;
        IRenderer* renderer_;
    };
}
