#include "Font.h"
#include <BinaryData.h>

using namespace juce;

namespace rp::uicore
{
    juce::Font& getRobotoCondensed()
    {
        static auto robotoCondensed (juce::Font (Typeface::createSystemTypefaceFor (BinaryData::RobotoCondensed_ttf,
                                                                          BinaryData::RobotoCondensed_ttfSize)));
        return robotoCondensed;
    }
}
