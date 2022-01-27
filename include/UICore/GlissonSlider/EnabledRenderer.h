#include "IRenderer.h"
#include "Knob.h"

namespace rp::uicore::glisson
{
    class EnabledRenderer : public IRenderer
    {
    public:
        EnabledRenderer(const std::vector<std::unique_ptr<Knob>>& knob);

        ~EnabledRenderer() override = default;

        void paint(juce::Graphics& g, const juce::Rectangle<float>& bounds) override;

    private:
        const std::vector<std::unique_ptr<Knob>>& knobs_;
    };
}
