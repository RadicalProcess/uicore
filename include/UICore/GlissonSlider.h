#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore
{
    class GlissonSlider : public juce::Component
    {
    public:
        class Knob
        {
        public:
            Knob(float initial, std::function<juce::Point<float>(float)>  converter);

            void setValue(float value);

            const juce::Point<float>& getPoint() const;

            bool isClose(const juce::Point<float>& point) const;

        private:
            float value_;
            juce::Point<float> point_;
            std::function<juce::Point<float>(float)> converter_;
        };


        GlissonSlider();

    private:


        void paint(juce::Graphics& g) override;


        void mouseDown(const juce::MouseEvent& event) override;

        void mouseDrag(const juce::MouseEvent& event) override;

        void mouseUp(const juce::MouseEvent& event) override;

        Knob startPositionKnobA_;

    };
}
