#include "Font.h"
#include <BinaryData.h>

using namespace juce;

namespace rp::uicore
{
    juce::Font& getJosefinSans()
    {
        static auto josefinSans (juce::Font (Typeface::createSystemTypefaceFor (BinaryData::JosefinSansBold_ttf,
                                                                          BinaryData::JosefinSansBold_ttfSize)));
        return josefinSans;
    }
}
