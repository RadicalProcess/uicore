#include "IRenderer.h"
#include "Knob.h"

namespace rp::uicore::glisson
{
    class DisabledRenderer : public IRenderer
    {
    public:
        DisabledRenderer(const std::vector<std::unique_ptr<Knob>>& knob);

        ~DisabledRenderer() override = default;

        void paint(juce::Graphics& g, const juce::Rectangle<float>& bounds) override;

    private:
        const std::vector<std::unique_ptr<Knob>>& knobs_;
    };
}
