#include "ComboBox.h"
#include "Font.h"
#include "Style.h"

namespace rp::uicore
{
    class ComboBoxLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        ComboBoxLookAndFeel()
        {
            setColour(juce::PopupMenu::backgroundColourId, juce::Colours::black);
            setColour(juce::ComboBox::textColourId, styles::text);
            setColour(juce::ComboBox::backgroundColourId, juce::Colours::black);
            setColour(juce::ComboBox::outlineColourId, juce::Colours::white);
            setColour(juce::ComboBox::buttonColourId, juce::Colours::black);
            setColour(juce::ComboBox::arrowColourId, styles::text);
            setColour(juce::ComboBox::focusedOutlineColourId, styles::highlight);
        }

        void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive,
                               bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text,
                               const juce::String& shortcutKeyText, const juce::Drawable* icon,
                               const juce::Colour* ) override
        {
            using namespace juce;
            auto font = getRobotoCondensed();
            g.setFont(font);

            if (isSeparator)
            {
                auto r = area.reduced(5, 0);
                r.removeFromTop(roundToInt(((float) r.getHeight() * 0.5f) - 0.5f));

                g.setColour(findColour(PopupMenu::textColourId).withAlpha(0.3f));
                g.fillRect(r.removeFromTop(1));
            }
            else
            {
                const auto textColour = juce::Colours::white;

                auto r = area.reduced(1);

                if (isHighlighted && isActive)
                {
                    g.setColour(findColour(PopupMenu::highlightedBackgroundColourId));
                    g.fillRect(r);

                    g.setColour(findColour(PopupMenu::highlightedTextColourId));
                }
                else
                {
                    g.setColour(textColour.withMultipliedAlpha(isActive ? 1.0f : 0.5f));
                }

                r.reduce(jmin(5, area.getWidth() / 20), 0);


                auto maxFontHeight = (float) r.getHeight() / 1.3f;

                if (font.getHeight() > maxFontHeight)
                    font.setHeight(maxFontHeight);

                g.setFont(font);

                auto iconArea = r.removeFromLeft(roundToInt(maxFontHeight)).toFloat();

                if (icon != nullptr)
                {
                    icon->drawWithin(g, iconArea, RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
                                     1.0f);
                    r.removeFromLeft(roundToInt(maxFontHeight * 0.5f));
                }
                else if (isTicked)
                {
                    auto tick = getTickShape(1.0f);
                    g.fillPath(tick, tick.getTransformToScaleToFit(
                                         iconArea.reduced(iconArea.getWidth() / 5, 0).toFloat(), true));
                }

                r.removeFromRight(3);
                g.drawFittedText(text, r, Justification::centredLeft, 1);
            }
        }

        void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY,
                          int buttonW, int buttonH, juce::ComboBox& box) override
        {
            using namespace juce;
            const auto boxBounds = Rectangle(0, 0, width, height);
            g.setColour(box.findColour(ComboBox::backgroundColourId));
            g.fillRect(boxBounds.toFloat());

            g.setColour(box.findColour(ComboBox::outlineColourId));
            g.drawRect(boxBounds.toFloat().reduced(1.5f, 3.0f), 0.4f);

            const auto arrowZone = Rectangle(width - 25, 0, 15, height);
            Path path;
            path.startNewSubPath((float) arrowZone.getX() + 3.0f, (float) arrowZone.getCentreY() - 2.0f);
            path.lineTo((float) arrowZone.getCentreX(), (float) arrowZone.getCentreY() + 3.0f);
            path.lineTo((float) arrowZone.getRight() - 3.0f, (float) arrowZone.getCentreY() - 2.0f);

            g.setColour(box.findColour(ComboBox::arrowColourId).withAlpha((box.isEnabled() ? 0.9f : 0.2f)));
            g.strokePath(path, PathStrokeType(1.0f));
        }

        juce::Font getComboBoxFont(juce::ComboBox&) override
        {
            return getRobotoCondensed();
        }
    };

    ComboBox::ComboBox(const std::string& name)
    : juce::ComboBox(name)
    , lf_(std::make_unique<ComboBoxLookAndFeel>())
    {
        setLookAndFeel(lf_.get());
        setJustificationType(juce::Justification::Flags::centred);
    }

    ComboBox::~ComboBox()
    {
        setLookAndFeel(nullptr);
    }
}
