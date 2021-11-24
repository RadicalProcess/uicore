#pragma once

#include <iomanip>

namespace rp::uicore
{
    inline
    std::string reduceNumDecimals(float value, int numDecimals)
    {
        auto stream = std::stringstream();
        stream << std::fixed << std::setprecision(numDecimals) << value;
        return stream.str();
    }
}