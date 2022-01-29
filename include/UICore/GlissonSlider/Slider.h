#pragma once

#include <set>
#include <juce_gui_basics/juce_gui_basics.h>

#include "KnobId.h"
#include "Knob.h"
#include "DisabledRenderer.h"
#include "EnabledRenderer.h"

namespace rp::uicore::glisson
{
    class Slider : public juce::Component
    {
    public:

        class Listener
        {
        public:
            virtual ~Listener() = default;

            virtual void onSliderValueChanged(KnobId knobId, float value) = 0;
        };

    public:
        Slider();

        void set(KnobId knobId, float value);

        void setGlissonEnabled(bool enabled);

        void addListener(Listener* listener);

        void removeListener(Listener* listener);

    private:

        void resized() override;

        void paint(juce::Graphics& g) override;

        void mouseDown(const juce::MouseEvent& event) override;

        void mouseDrag(const juce::MouseEvent& event) override;

        void mouseUp(const juce::MouseEvent& event) override;

        std::set<Listener*> listeners_;
        std::vector<std::unique_ptr<Knob>> knobs_;
        EnabledRenderer enabledRenderer_;
        DisabledRenderer disabledRenderer_;
        IRenderer* renderer_;
    };
}
