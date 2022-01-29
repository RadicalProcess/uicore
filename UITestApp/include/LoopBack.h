#pragma once

#include <UICore/GlissonSlider/Slider.h>

namespace rp::uitest
{
    class LoopBack : public uicore::glisson::Slider::Listener
    {
    public:
        explicit LoopBack(uicore::glisson::Slider& slider)
        : slider_(slider){
            slider_.addListener(this);
        }

        ~LoopBack() override
        {
            slider_.removeListener(this);
        }

        void onSliderValueChanged(uicore::glisson::KnobId knobId, float value) override
        {
            slider_.set(knobId, value);
        }

        uicore::glisson::Slider& slider_;
    };
}
